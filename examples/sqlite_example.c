#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sqlite3.h"
#include "mongoose.h"

/*
用fast-fail风格,最大的问题是return的时候很容易漏掉资源回收语句
就算没有漏, return越多, 重复的资源回收语句也越多, 因为C没有try-catch
解决重复写这些语句的方法
1. 用变量和goto代替return
2. 用宏模拟实现try-catch-finally
*/

//
// /////////////////////////////////////////////////////////////////////////////
//

enum
{
    SQLIITE_GET,
    SQLIITE_SELECT,
    SQLIITE_INSERT,
    SQLIITE_UPDATE,
    SQLIITE_UPSERT,
    SQLIITE_DELELE
};

//
// /////////////////////////////////////////////////////////////////////////////
//

/** 打开sqlite数据库 */
sqlite3 *
sqlite_open(
    const char *db_path
);

/** 关闭sqlite数据库 */
void sqlite_close(
    sqlite3 *db
);

/** 操作 */
int sqlite_op(
    sqlite3 *db,
    int op,
    const struct mg_str *key,
    struct mg_str *val
);

static int sqlite_get(sqlite3 *db, const struct mg_str *key, char* *val);
static int sqlite_set(sqlite3 *db, const struct mg_str *key, const struct mg_str *val);
static int sqlite_del(sqlite3 *db, const struct mg_str *key);

static char * mg_str2str(struct mg_str *mgstr);
static char * string_copy(char * str);

//
// /////////////////////////////////////////////////////////////////////////////
//
int main(int argc, char* argv[])
{
    char* path = argc > 1 ? argv[1] : "sqlite_example.db";
    char* key_str = argc > 2 ? argv[2] : "key";
    char* val_str = argc > 3 ? argv[3] : "val";

    struct mg_str key = mg_mk_str(key_str);
    
    // open
    sqlite3 *db = sqlite_open(path);
    
    // get
    {
        char* val = NULL;
        sqlite_get(db, &key, &val);
        printf("GET %s=%s\n", key_str, val);
        free(val);
    }

    // set
    {
        struct mg_str val = mg_mk_str(val_str);
        sqlite_set(db, &key, &val);
        printf("SET %s=%s\n", key_str, val_str);
    }

    // get
    {
        char* val = NULL;
        sqlite_get(db, &key, &val);
        printf("GET %s=%s\n", key_str, val);
        free(val);
    }

    // del
    {
        sqlite_del(db, &key);
        printf("DEL %s\n", key_str);
    }

    // get
    {
        char* val = NULL;
        sqlite_get(db, &key, &val);
        printf("GET %s=%s\n", key_str, val);
        free(val);
    }

    // close
    sqlite3_close(db);
}

sqlite3 *sqlite_open(const char *db_path)
{
    sqlite3 *db = NULL;
    if (sqlite3_open_v2(
            db_path,
            &db,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
            NULL
        ) == SQLITE_OK
    ){
        sqlite3_exec(
            db,
            "CREATE TABLE IF NOT EXISTS kv(key PRIMARY KEY, val);",
            0,
            0,
            0
        );
    }
    return db;
}

void sqlite_close(sqlite3 *db)
{
    if (db != NULL){
        sqlite3_close(db);
    }
}

int sqlite_op(
    sqlite3 *db,
    int op,
    const struct mg_str *key,
    struct mg_str *val
){
    int code = 0;
    switch (op)
    {
    case SQLIITE_GET: {
        char* v = NULL;
        sqlite_get(db, key, &v);
        if (v!=NULL) {
            struct mg_str *mgstr = malloc(sizeof(struct mg_str));
            char* tmp = (char*) (mgstr->p);
            tmp = v;
            mgstr->len = strlen(v);
            val = mgstr;
        } else {
            val = NULL;
        }

        break;
    }
    case SQLIITE_SELECT: {
        printf("Not Support Select Operation.");
        code = -1;
        break;
    }
    case SQLIITE_INSERT:
        printf("Not Support Insert Operation, use Upsert instead.");
        code = -1;
        break;
    case SQLIITE_UPDATE:
        printf("Not Support Update Operation, use Upsert instead.");
        code = -1;
        break;
    case SQLIITE_UPSERT:
        sqlite_set(db, key, val);
        break;
    case SQLIITE_DELELE:
        sqlite_del(db, key);
        break;
    default:
        printf("Unkown Operation Code: %d", op);
        code = -2;
        break;
    }
    return code;
}

static int sqlite_set(sqlite3 *db, const struct mg_str *key, const struct mg_str *val)
{
    // 查询语句
    const char* sql = "INSERT OR REPLACE INTO kv VALUES (?, ?);";
    // 预编译语句
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt,NULL) != SQLITE_OK){
        return 500;
    }

    sqlite3_bind_text(stmt, 1, key->p, key->len, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, val->p, val->len, SQLITE_STATIC);

    sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);

    return 200;
}

static int sqlite_get(sqlite3 *db, const struct mg_str *key, char* *val)
{
    int code;
    // 查询语句
    const char* sql = "SELECT val FROM kv WHERE key = ?;";
    // 预编译语句
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt,NULL) != SQLITE_OK){
        code = 500;
        goto FINALLY;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, key->p, key->len, SQLITE_STATIC);

    // 执行语句
    code = sqlite3_step(stmt);
    if (code != SQLITE_OK && code != SQLITE_ROW) {
        code = 500;
        goto FINALLY;
    }
    
    // 获取第一列的值
    char* v = (char*)sqlite3_column_text(stmt, 0);
    if(v == NULL){
        code = 404;
        goto FINALLY;
    }

    // 赋值
    *val = string_copy(v); 
    // printf("sqlite_get v=%s, val=%s\n", v, *val);

    code = 200;
    
    FINALLY:
        sqlite3_finalize(stmt);
        // TODO: v需要释放内存吗?
        // TODO: v在释放了stmt之后, 它的内存也跟着被回收了, 所以需要在关闭stmt之前复制一份出来
    RETURN:
        return code;
}

static int sqlite_del(sqlite3 *db, const struct mg_str *key)
{
    // 查询语句
    const char* sql = "DELETE FROM kv WHERE key = ?;";
    // 预编译语句
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt,NULL) != SQLITE_OK){
        return 500;
    }

    sqlite3_bind_text(stmt, 1, key->p, key->len, SQLITE_STATIC);
    int result = sqlite3_step(stmt);
    if (result != SQLITE_OK && result != SQLITE_ROW)
    {
        return 404;
    }

    sqlite3_finalize(stmt);


    return 200;

}

static char * mg_str2str(struct mg_str *mgstr)
{
    if(mgstr==NULL || mgstr->p==NULL || mgstr->len==0){
        return NULL;
    }

    char * text = (char *) malloc(mgstr->len + 1);
    strncpy(text, mgstr->p, mgstr->len);
    text[mgstr->len] = '\0';
    return text;
}


static char * string_copy(char * str)
{
    if(str==NULL){
        return NULL;
    }
    int len = strlen(str);
    char * text = (char *) malloc(len + 1);
    strncpy(text, str, len);
    text[len] = '\0';
    return text;
}
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
    const char *db_path);

/** 关闭sqlite数据库 */
void sqlite_close(
    void **db_handle);

/** 操作 */
int sqlite_op(
    struct mg_connection *nc,
    const struct http_message *hm,
    const struct mg_str *key,
    void *db,
    int op);

//
// /////////////////////////////////////////////////////////////////////////////
//

sqlite3 *sqlite_open(const char *db_path)
{
    sqlite3 *db = NULL;
    if (sqlite3_open_v2(
            db_path,
            &db,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
            NULL) == SQLITE_OK)
    {
        sqlite3_exec(
            db,
            "CREATE TABLE IF NOT EXISTS kv(key PRIMARY KEY, val);",
            0,
            0,
            0);
    }
    return db;
}

void sqlite_close(void **db_handle)
{
    if (db_handle != NULL && *db_handle != NULL)
    {
        sqlite3_close(*db_handle);
        *db_handle = NULL;
    }
}

int sqlite_op(
    struct mg_connection *nc,
    const struct http_message *hm,
    const struct mg_str *key,
    void *db,
    int op)
{
    int code = 0;
    switch (op)
    {
    case SQLIITE_SELECT:
        op_get(nc, hm, key, db);
        break;
    case SQLIITE_INSERT:
        break;
    case SQLIITE_UPDATE:
        break;
    case SQLIITE_UPSERT:
        op_set(nc, hm, key, db);
        break;
    case SQLIITE_DELELE:
        op_del(nc, hm, key, db);
        break;
    default:
        mg_printf(nc, "%s",
                  "HTTP/1.0 501 Not Implemented\r\n"
                  "Content-Length: 0\r\n\r\n");
        break;
    }
    return code;
}

static int sqlite_op_set(sqlite3 *db, const char *key, int key_len, char *value, int value_len)
{
    // 查询语句
    const char* sql = "INSERT OR REPLACE INTO kv VALUES (?, ?);";
    // 预编译语句
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt,NULL) == SQLITE_OK){
        return 500;
    }

    sqlite3_bind_text(stmt, 1, key, key_len, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, value, value_len, SQLITE_STATIC);

    sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);

    return 200;
}

static int sqlite_op_get(sqlite3 *db, const char *key, int len, char **value)
{
    int code;
    // 查询语句
    const char* sql = "SELECT val FROM kv WHERE key = ?;";
    // 预编译语句
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt,NULL) == SQLITE_OK){
        code = 500;
        goto FINALLY;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, key, len, SQLITE_STATIC);

    // 执行语句
    code = sqlite3_step(stmt);
    if (code != SQLITE_OK && code != SQLITE_ROW) {
        code = 500;
        goto FINALLY;
    }
    
    // 获取第一列的值
    const char *data = (char *)sqlite3_column_text(stmt, 0);
    if(data == NULL){
        code = 404;
        goto FINALLY;
    }

    // 赋值
    *value = data; // (int) strlen(data), data
    code = 200;
    
    FINALLY:
        sqlite3_finalize(stmt);
    RETURN:
        return code;
}

static int sqlite_op_del(sqlite3 *db, const char *key, int len)
{
    // 查询语句
    const char* sql = "DELETE FROM kv WHERE key = ?;";
    // 预编译语句
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt,NULL) == SQLITE_OK){
        return 500;
    }

    sqlite3_bind_text(stmt, 1, key, len, SQLITE_STATIC);
    int result = sqlite3_step(stmt);
    if (result != SQLITE_OK && result != SQLITE_ROW) {
    {
        return 404;
    }

    sqlite3_finalize(stmt);


    return 200;

}

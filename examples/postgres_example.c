#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libpq-fe.h>

/* for ntohl/htonl , 用于网络字节序转换 */
#include <netinet/in.h>
#include <arpa/inet.h>

//
// /////////////////////////////////////////////////////////////////////////////
//

//
// /////////////////////////////////////////////////////////////////////////////
//

static void print_result(PGresult *rs);
static void exit_nicely(PGconn *conn);









//
// /////////////////////////////////////////////////////////////////////////////
//
int main(int argc, char* argv[])
{
    // postgresql://[user[:password]@][netloc][:port][,...][/dbname][?param1=value1&...]
    const char *conninfo = "dbname = postgres";
    PGconn* conn = PQconnectdb(conninfo);
    {
        PGresult* rs;

        const char * sql = 
            "SELECT             "
            "    name,          "
            "    category,      "
            "    setting,       "
            "    unit,          "
            "    short_desc     "
            "FROM               "
            "    pg_settings    "
            "ORDER BY           "
            "    category       "
            ;
        rs = PQexec(conn, sql);
        if (PQstatus(conn) != CONNECTION_OK){
            fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
            exit_nicely(conn);
        }

        /* 设置总是安全的搜索路径，这样恶意用户就无法取得控制。 */
        rs = PQexec(conn, sql);
        switch(PQresultStatus(rs)){
        // a query command that doesn't return anything was executed properly by the backend 
        case PGRES_COMMAND_OK:
            fprintf(stderr, "execute success, affected rows: %s", PQcmdTuples(rs));
            PQclear(rs);
            break;
        // a query command that returns tuples was executed properly by the backend, PGresult contains the result tuples
        case PGRES_TUPLES_OK:
            print_result(rs);
            PQclear(rs);
            break;
        case PGRES_COPY_OUT:			/* Copy Out data transfer in progress */
        case PGRES_COPY_IN:				/* Copy In data transfer in progress */
        case PGRES_COPY_BOTH:			/* Copy In/Out data transfer in progress */
        case PGRES_SINGLE_TUPLE:		/* single tuple from larger resultset */
            fprintf(stderr, "not support this sql now: %s", sql);
            PQclear(rs);
            exit_nicely(conn);
            break;
        case PGRES_EMPTY_QUERY:	/* empty query string was executed */
        case PGRES_BAD_RESPONSE:			/* an unexpected response was recv'd from the backend */
        case PGRES_NONFATAL_ERROR:		/* notice or warning message */
        case PGRES_FATAL_ERROR:			/* query failed */
        default:
            fprintf(stderr, "SET failed: %s", PQerrorMessage(conn));
            PQclear(rs);
            exit_nicely(conn);
        }
    }
    PQfinish(conn);

    return 0;
}

static void print_result(PGresult *rs)
{
    int row,
        col,
        rows = PQntuples(rs),
        cols = PQnfields(rs)
    ;

    char* name;
    char* value;

    // 打印列名,类型
    for (col = 0; col < cols; col++) {
        printf("%s\t", PQfname(rs, col));
    }
    printf("\n");

    // 打印行
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            value = PQgetvalue(rs, row, col);
            printf("%s\t", value);
        }
        printf("\n");
    }
    printf("\nrows: %d\n", rows);
}

static void exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

// int PQbackendPID(const PGconn *conn);

// char *PQescapeLiteral(PGconn *conn, const char *str, size_t length); // 对字符串做转义, 譬如单引号,反斜杠
// PGresult *PQexec(PGconn *conn, const char *command);

// ExecStatusType PQresultStatus(const PGresult *res);
// char *PQresStatus(ExecStatusType status);
// char *PQresultErrorMessage(const PGresult *res);

// void PQclear(PGresult *res);


// int PQntuples(const PGresult *res);
// int PQnfields(const PGresult *res);
// char *PQfname(const PGresult *res, int column_number);
// int PQfnumber(const PGresult *res, const char *column_name);
// Oid PQftable(const PGresult *res, int column_number); // 列来自哪个表, 有可能返回InvalidOid常量
// int PQfformat(const PGresult *res, int column_number);
// Oid PQftype(const PGresult *res, int column_number);
// int PQfsize(const PGresult *res, int column_number);
// char *PQgetvalue(const PGresult *res, int row_number, int column_number);

/*
PQgetvalue取得字符串后的类型转换

PQgetvalue返回的是以\0结束的字符串，将其转换成int即可。
char* res = PQgetvalue(....);
判断一下res是否为空，或者调用PQgetisnull()先。
int n = atoi(res);
有的平台可能还有字节序问题, 那就要用ntohl了

 */

// int PQgetisnull(const PGresult *res, int row_number, int column_number);
// int PQgetlength(const PGresult *res, int row_number, int column_number);

// char *PQcmdTuples(PGresult *res); // 命令影响的行数





/*
34.4. 异步命令处理

PQexec函数对于在普通的同步应用中提交命令是足以胜任的。不过，它的一些缺点可能对某些用户很重要：

PQexec会等待命令完成。该应用可能有其他的工作要做（例如维护用户界面），这时它将不希望阻塞等待回应。

因为客户端应用的执行在它等待结果时会被挂起，对于应用来说很难决定要不要尝试取消正在进行的命令（这可以在一个信号处理器中完成，但别无他法）。

PQexec只能返回一个PGresult结构。如果提交的命令串包含多个SQL命令， 除了最后一个PGresult之外都会被PQexec丢弃。

PQexec总是收集命令的整个结果，把它缓存在一个单一的PGresult中。虽然这简化了应用的错误处理逻辑，它对于包含很多行的结果并不现实。

不想受到这些限制的应用可以改用构建PQexec的底层函数：PQsendQuery以及PQgetResult。
还有 PQsendQueryParams、 PQsendPrepare、 PQsendQueryPrepared、 PQsendDescribePrepared以及 PQsendDescribePortal， 
它们可以与PQgetResult一起使用来分别复制PQexecParams、 PQprepare、 PQexecPrepared、 PQdescribePrepared和 PQdescribePortal的功能。
 */
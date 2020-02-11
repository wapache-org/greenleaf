
// /* 要求 libpq 事件的头文件（注意：包括 libpq-fe.h） */
// #include <libpq-events.h>

// /* instanceData */
// typedef struct
// {
//     int n;
//     char *str;
// } mydata;

// /* PGEventProc */
// static int myEventProc(PGEventId evtId, void *evtInfo, void *passThrough);

// int
// main111(void)
// {
//     mydata *data;
//     PGresult *res;
//     PGconn *conn =
//         PQconnectdb("dbname=postgres options=-csearch_path=");

//     if (PQstatus(conn) != CONNECTION_OK)
//     {
//         fprintf(stderr, "Connection to database failed: %s",
//                 PQerrorMessage(conn));
//         PQfinish(conn);
//         return 1;
//     }

//     /* 在任何应该接收事件的连接上调用一次。
//      * 发送一个 PGEVT_REGISTER 给 myEventProc。
//      */
//     if (!PQregisterEventProc(conn, myEventProc, "mydata_proc", NULL))
//     {
//         fprintf(stderr, "Cannot register PGEventProc\n");
//         PQfinish(conn);
//         return 1;
//     }

//     /* conn 的 instanceData 可用 */
//     data = PQinstanceData(conn, myEventProc);

//     /* 发送一个 PGEVT_RESULTCREATE 给 myEventProc */
//     res = PQexec(conn, "SELECT 1 + 1");

//     /* 结果的 instanceData 可用 */
//     data = PQresultInstanceData(res, myEventProc);

//     /* 如果使用了 PG_COPYRES_EVENTS，发送一个 PGEVT_RESULTCOPY 给 myEventProc */
//     PGresult* res_copy = PQcopyResult(res, PG_COPYRES_TUPLES | PG_COPYRES_EVENTS);

//     /* 如果在 PQcopyResult 调用时使用了 PG_COPYRES_EVENTS，结果的 instanceData 可用。*/
//     data = PQresultInstanceData(res_copy, myEventProc);

//     /* 两个清除都发送一个 PGEVT_RESULTDESTROY 给 myEventProc */
//     PQclear(res);
//     PQclear(res_copy);

//     /* 发送一个 PGEVT_CONNDESTROY 给 myEventProc */
//     PQfinish(conn);

//     return 0;
// }

// static int
// myEventProc(PGEventId evtId, void *evtInfo, void *passThrough)
// {
//     switch (evtId)
//     {
//         case PGEVT_REGISTER:
//         {
//             PGEventRegister *e = (PGEventRegister *)evtInfo;
//             mydata *data = get_mydata(e->conn);

//             /* 将应用相关的数据与连接关联起来 */
//             PQsetInstanceData(e->conn, myEventProc, data);
//             break;
//         }

//         case PGEVT_CONNRESET:
//         {
//             PGEventConnReset *e = (PGEventConnReset *)evtInfo;
//             mydata *data = PQinstanceData(e->conn, myEventProc);

//             if (data)
//               memset(data, 0, sizeof(mydata));
//             break;
//         }

//         case PGEVT_CONNDESTROY:
//         {
//             PGEventConnDestroy *e = (PGEventConnDestroy *)evtInfo;
//             mydata *data = PQinstanceData(e->conn, myEventProc);

//             /* 因为连接正在被销毁，释放示例数据 */
//             if (data)
//               free_mydata(data);
//             break;
//         }

//         case PGEVT_RESULTCREATE:
//         {
//             PGEventResultCreate *e = (PGEventResultCreate *)evtInfo;
//             mydata *conn_data = PQinstanceData(e->conn, myEventProc);
//             mydata *res_data = dup_mydata(conn_data);

//             /* 把应用相关的数据与结果（从 conn 复制过来）关联起来 */
//             PQsetResultInstanceData(e->result, myEventProc, res_data);
//             break;
//         }

//         case PGEVT_RESULTCOPY:
//         {
//             PGEventResultCopy *e = (PGEventResultCopy *)evtInfo;
//             mydata *src_data = PQresultInstanceData(e->src, myEventProc);
//             mydata *dest_data = dup_mydata(src_data);

//             /* 把应用相关的数据与结果（从一个结果复制过来）关联起来 */
//             PQsetResultInstanceData(e->dest, myEventProc, dest_data);
//             break;
//         }

//         case PGEVT_RESULTDESTROY:
//         {
//             PGEventResultDestroy *e = (PGEventResultDestroy *)evtInfo;
//             mydata *data = PQresultInstanceData(e->result, myEventProc);

//             /* 因为结果正在被销毁，释放实例数据 */
//             if (data)
//               free_mydata(data);
//             break;
//         }

//         /* 未知事件 ID，只返回true。 */
//         default:
//             break;
//     }

//     return true; /* 事件处理成功 */
// }
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "mongoose.h"

#include <libpq-fe.h>

#include <quickjs/quickjs.h>
#include <quickjs/quickjs-libc.h>


static const char *s_http_port = "8000";

static const struct mg_str s_get_method = MG_MK_STR("GET");
static const struct mg_str s_put_method = MG_MK_STR("PUT");
static const struct mg_str s_delele_method = MG_MK_STR("DELETE");


static int s_sig_num = 0;
static struct mg_serve_http_opts s_http_server_opts;


static void signal_handler(int sig_num);
static void event_handler(struct mg_connection *nc, int ev, void *ev_data);

static void handle_api_request(struct mg_connection *nc, struct http_message *hm);
static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix);
static int is_equal(const struct mg_str *s1, const struct mg_str *s2);


#define DEFAULT_JS_OBJECT_CLASS_ID 1
static JSRuntime *runtime;
static JSContext *context;
static JSValue grobal;

static int qjs_runtime_init();
static int qjs_runtime_free();

static JSValue js_loadScript(JSContext *ctx, const char *filename);
static uint8_t *load_file(JSContext *ctx, size_t *pbuf_len, const char *filename);
static void print_exception(JSContext *ctx, JSValue e);
static void print_exception_free(JSContext *ctx, JSValue e);
static int is_exception_free(JSContext *ctx, JSValue e);
static int if_is_exception_then_free(JSContext *ctx, JSValue e);
static void print_value(JSContext *ctx, JSValue e);
static void print_property(JSContext *ctx, JSValue this_obj, const char* property_name);


static JSValue js_PQconnectdb(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_PQfinish(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static JSValue js_PQexec(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_PQprintresult(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static JSValue js_PQntuples(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_PQnfields(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_PQfname(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_PQfnumber(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static JSValue js_PQgetvalue(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_PQclear(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

// ////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    char* hexdump_file;
    char* document_root = "./";

    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-D") == 0) {
            hexdump_file = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0) {
            document_root = argv[++i];
        }
    }

    /* Open listening socket */
    struct mg_mgr mgr;
    mg_mgr_init(&mgr, NULL);{
        if(hexdump_file!=NULL){
            mgr.hexdump_file = hexdump_file;
        }
    }

    s_http_server_opts.document_root = document_root;

    struct mg_connection *nc = mg_bind(&mgr, s_http_port, event_handler);
    mg_set_protocol_http_websocket(nc);

    // 初始化js runtime
    qjs_runtime_init();

    /* Run event loop until signal is received */
    printf("Starting RESTful server on port %s\n", s_http_port);
    while (s_sig_num == 0) {
        mg_mgr_poll(&mgr, 1000);
    }

    /* Cleanup */
    mg_mgr_free(&mgr);

    qjs_runtime_free();

    printf("Exiting on signal %d\n", s_sig_num);

    return 0;
}

static void signal_handler(int sig_num) {
    signal(sig_num, signal_handler);
    s_sig_num = sig_num;
}

static const struct mg_str api_prefix = MG_MK_STR("/api/v1/");
static void event_handler(struct mg_connection *nc, int ev, void *ev_data) 
{
    struct http_message *hm = (struct http_message *) ev_data;

    switch (ev) {
    case MG_EV_HTTP_REQUEST:
        if (has_prefix(&hm->uri, &api_prefix)) 
        {
            handle_api_request(nc, hm);
        } else {
            mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
        }
        break;
    }
}

static void handle_api_request(struct mg_connection *nc, struct http_message *hm)
{
    // new request
    JSValue request = JS_NewObject(context);{

        JS_SetOpaque(request, hm);
        // struct http_message *hm = JS_GetOpaque(request, DEFAULT_JS_OBJECT_CLASS_ID);

        char method[hm->method.len+1];
        memset( method, 0, sizeof(method));
        memcpy(method, hm->method.p, hm->method.len);

        char uri[hm->uri.len+1];
        memset( uri, 0, sizeof(uri));
        memcpy(uri, hm->uri.p, hm->uri.len);

        char path[sizeof(uri)-api_prefix.len+1];
        memset( path, 0, sizeof(path));
        memcpy(path, uri+(api_prefix.len-1), sizeof(path)-1);

        // hm->query_string
        char query_string[hm->query_string.len+1];
        memset( query_string, 0, sizeof(query_string));
        memcpy(query_string, hm->query_string.p, hm->query_string.len);

        JS_SetPropertyStr(context, request, "method", JS_NewString(context, method));
        JS_SetPropertyStr(context, request, "uri", JS_NewString(context, uri));
        JS_SetPropertyStr(context, request, "path", JS_NewString(context, path));
        JS_SetPropertyStr(context, request, "query_string", JS_NewString(context, query_string));

    }

    // new response
    JSValue response = JS_NewObject(context);{
        JS_SetPropertyStr(context, response, "status", JS_NewInt32(context, 404));
        JS_SetPropertyStr(context, response, "status_text", JS_NewString(context, "Not Found"));
        JS_SetPropertyStr(context, response, "headers", JS_NewArray(context));
        JS_SetPropertyStr(context, response, "body", JS_NULL);
    }

    // handle request
    {
        JSAtom fn_name = JS_NewAtom(context, "handle");
        JSValue argv2[] = { request, response };
        JSValue rs = JS_Invoke(context, grobal, fn_name, 2, argv2);
        JS_FreeAtom(context, fn_name);

        if(if_is_exception_then_free(context, rs)) goto clean;
        JS_FreeValue(context, rs);
    }

    // send response
    {
        JSValue status_value = JS_GetPropertyStr(context, response, "status");
        JSValue status_text_value = JS_GetPropertyStr(context, response, "status_text");
        JSValue headers_value = JS_GetPropertyStr(context, response, "headers");
        JSValue body_value = JS_GetPropertyStr(context, response, "body");

        int32_t status;JS_ToInt32(context, &status, status_value);
        const char* status_text = JS_ToCString(context, status_text_value);
        
        const char* body = JS_IsNull(body_value) ? "" : JS_ToCString(context, body_value);

        char* headers = "";
        
        mg_printf(nc, 
            "HTTP/1.0 %d %s\r\n" // response line
            "Content-Length: %d\r\n"
            "Content-Type: application/json;charset=utf-8\r\n"
            "%s" // headers
            "\r\n"
            "%s", // body
            status, status_text==NULL?"":status_text,
            strlen(body),
            headers,
            body
        );
        
    }

    clean:
        JS_SetOpaque(request, NULL);
        JS_FreeValue(context, request);
        JS_FreeValue(context, response);
    
}

static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix) {
    return uri->len > prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static int is_equal(const struct mg_str *s1, const struct mg_str *s2) {
    return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
}



// ////////////////////////////////////////////////////////////////////////////

static int qjs_runtime_init() 
{

  runtime = JS_NewRuntime();
  context = JS_NewContext(runtime);
  grobal = JS_GetGlobalObject(context);

  js_std_add_helpers(context, 0, NULL);

  js_loadScript(context, "qjs_mongoose_template_engine.js"); // 模板引擎
  js_loadScript(context, "qjs_mongoose_handler.js"); // api 请求处理脚本

  JS_SetPropertyStr(context, grobal, "pg_connect_db", JS_NewCFunction(context, js_PQconnectdb, NULL, 0));

  return 0;
}


static int qjs_runtime_free() 
{
  JS_FreeValue(context, grobal);
  JS_FreeContext(context);
  JS_FreeRuntime(runtime);
  return 0;
}


/* load and evaluate a file */
static JSValue js_loadScript(JSContext *ctx, const char *filename)
{
    uint8_t *buf;
    JSValue ret;
    size_t buf_len;
    
    if (!filename)
        return JS_EXCEPTION;
    buf = load_file(ctx, &buf_len, filename);
    if (!buf) {
        JS_ThrowReferenceError(ctx, "could not load '%s'", filename);
        return JS_EXCEPTION;
    }
    ret = JS_Eval(ctx, (char *)buf, buf_len, filename, JS_EVAL_TYPE_GLOBAL);
    js_free(ctx, buf);
    return ret;
}

static uint8_t *load_file(JSContext *ctx, size_t *pbuf_len, const char *filename)
{
    FILE *f;
    uint8_t *buf;
    size_t buf_len;
    long lret;
    
    // 获取文件句柄
    f = fopen(filename, "rb");
    if (!f)
        return NULL;

    // 获取文件长度
    if (fseek(f, 0, SEEK_END) < 0)
        goto fail;
    lret = ftell(f);
    if (lret < 0)
        goto fail;
    /* XXX: on Linux, ftell() return LONG_MAX for directories */
    if (lret == LONG_MAX) {
        errno = EISDIR;
        goto fail;
    }
    buf_len = lret;
    if (fseek(f, 0, SEEK_SET) < 0)
        goto fail;

    // 申请内存
    if (ctx)
        buf = js_malloc(ctx, buf_len + 1);
    else
        buf = malloc(buf_len + 1);
    if (!buf)
        goto fail;
    
    // 读取文件
    if (fread(buf, 1, buf_len, f) != buf_len) {
        errno = EIO;
        if (ctx)
            js_free(ctx, buf);
        else
            free(buf);
    fail:
        fclose(f);
        return NULL;
    }
    buf[buf_len] = '\0';

    // 关闭文件
    fclose(f);

    // 返回
    *pbuf_len = buf_len;
    return buf;
}

// ////////////////////////////////////////////////////////////////////////////

// PGconn* PQconnectdb(const char* conninfo);
static JSValue js_PQconnectdb(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    // if(argc<=0){

    // }

    // if(!argv[0]){
        
    // }

    const char *conninfo;

    conninfo = JS_ToCString(ctx, argv[0]);
    PGconn* conn = PQconnectdb(conninfo);
    //printf("conn address: %x\n", &conn);

    JSValue connObj = JS_NewObject(ctx);{
      JS_SetOpaque(connObj, (void*)conn);

      // 给这个object添加方法
      JS_SetPropertyStr(ctx, connObj, "__c_pointer_type", JS_NewString(ctx, "PGconn*"));
      JS_SetPropertyStr(ctx, connObj, "close", JS_NewCFunction(ctx, js_PQfinish, NULL, 0));
      // JS_SetPropertyStr(ctx, connObj, "execute", JS_NewCFunction(ctx, js_PQfinish, NULL, 0));
      JS_SetPropertyStr(ctx, connObj, "query", JS_NewCFunction(ctx, js_PQexec, NULL, 0));

      // JS_SetPropertyFunctionList(ctx, connObj, tab, 2);
    }
    
    // 验证是不是可以取回来指针
    // PGconn* conn2 = JS_GetOpaque(connObj, (JSClassID)NULL);


    clean:
        if(conninfo!=NULL) JS_FreeCString(ctx, conninfo);
    done:
        return connObj;

}

// void PQfinish(conn);
static JSValue js_PQfinish(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    // 检查它是不是连接对象
    // JSValue v = JS_GetPropertyStr(ctx, this_val, "__c_pointer_type");
    PGconn* conn = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    //printf("conn address 2: %x\n", &conn);
    PQfinish(conn);

}

//PGresult* js_PQexec(PGconn *conn, const char *command);
static JSValue js_PQexec(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *command = JS_ToCString(ctx, argv[0]);
    if (!command){
      JS_FreeCString(ctx, command);
      return JS_EXCEPTION;
    }
    PGconn* conn = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    PGresult* res = PQexec(conn, command);
    JS_FreeCString(ctx, command);

    JSValue resultObj = JS_NewObject(ctx);{
      JS_SetOpaque(resultObj, (void*)res);
          // 给这个object添加方法

      JS_SetPropertyStr(ctx, resultObj, "__c_pointer_type", JS_NewString(ctx, "PGresult*"));
      JS_SetPropertyStr(ctx, resultObj, "close", JS_NewCFunction(ctx, js_PQclear, NULL, 0));
      JS_SetPropertyStr(ctx, resultObj, "print", JS_NewCFunction(ctx, js_PQprintresult, NULL, 0));

      JS_SetPropertyStr(ctx, resultObj, "getRowCount", JS_NewCFunction(ctx, js_PQntuples, NULL, 0));
      JS_SetPropertyStr(ctx, resultObj, "getColumnCount", JS_NewCFunction(ctx, js_PQnfields, NULL, 0));
      JS_SetPropertyStr(ctx, resultObj, "getColumnName", JS_NewCFunction(ctx, js_PQfname, NULL, 0));
      JS_SetPropertyStr(ctx, resultObj, "getColumnIndex", JS_NewCFunction(ctx, js_PQfnumber, NULL, 0));

      JS_SetPropertyStr(ctx, resultObj, "getValue", JS_NewCFunction(ctx, js_PQgetvalue, NULL, 0));

      // JS_SetPropertyFunctionList(ctx, connObj, tab, 2);
    }


    clean:
        // if(command!=NULL) JS_FreeCString(ctx, command);
    done:
        return resultObj;

}

// int PQntuples(const PGresult *res);
static JSValue js_PQntuples(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    PGresult* res = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    if(res==NULL)
      return JS_EXCEPTION;

    int rows = PQntuples(res);
  return JS_NewInt32(ctx, rows);
}

// int PQnfields(const PGresult *res);
static JSValue js_PQnfields(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    PGresult* res = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    if(res==NULL)
      return JS_EXCEPTION;

    int cols = PQnfields(res);
  return JS_NewInt32(ctx, cols);
}

// char *PQfname(const PGresult *res, int column_number);
static JSValue js_PQfname(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    PGresult* res = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    if(res==NULL)
      return JS_EXCEPTION;

    int col = JS_VALUE_GET_INT(argv[0]);
    char* name = PQfname(res, col);
  return JS_NewString(ctx, name);
}

// int PQfnumber(const PGresult *res, const char *column_name);
static JSValue js_PQfnumber(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    PGresult* res = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    if(res==NULL)
      return JS_EXCEPTION;
      const char* name = JS_ToCString(ctx, argv[0]);
      int col = PQfnumber(res, name);
  return JS_UNDEFINED;
}

// char *PQgetvalue(const PGresult *res, int row_number, int column_number);

// void js_PQclear(PGresult *res)
static JSValue js_PQclear(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    PGresult* res = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    if(res!=NULL)
      PQclear(res);
  return JS_UNDEFINED;
}

//void js_PQprintresult(PGresult *res)
static JSValue js_PQprintresult(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    PGresult* res = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    if(res==NULL)
      return JS_EXCEPTION;
    
    int row,
        col,
        rows = PQntuples(res),
        cols = PQnfields(res)
    ;

    char* name;
    char* value;

    // 打印列名,类型
    for (col = 0; col < cols; col++) {
        printf("%s\t", PQfname(res, col));
    }
    printf("\n");

    // 打印行
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            value = PQgetvalue(res, row, col);
            printf("%s\t", value);
        }
        printf("\n");
    }
    printf("\nrows: %d\n", rows);

}

//char* js_PQgetvalue(const PGresult *res, int row_number, int column_number)
static JSValue js_PQgetvalue(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    if(argc<2)
      return JS_EXCEPTION;

    PGresult* res = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    if(res==NULL)
      return JS_EXCEPTION;

    int row = JS_VALUE_GET_INT(argv[0]);
    int col = JS_VALUE_GET_INT(argv[1]);

    char* val = PQgetvalue(res, row, col);
    return JS_NewString(ctx, val);
}


static void print_exception(JSContext *ctx, JSValue e)
{
  assert(JS_IsException(e));
  const char* msg = JS_ToCString(ctx, e);
  printf("ERROR: %s\n", msg);
  JS_FreeCString(ctx, msg);
}

static void print_exception_free(JSContext *ctx, JSValue e)
{
  print_exception(ctx, e);
  JS_FreeValue(ctx, e);
}

static void print_value(JSContext *ctx, JSValue e)
{
  const char* msg = JS_ToCString(ctx, e);
  printf("DUMP: %s\n", msg);
  JS_FreeCString(ctx, msg);
}

static void print_property(JSContext *ctx, JSValue this_obj, const char* property_name)
{
  JSAtom property_atom = JS_NewAtom(ctx, property_name);
  JSValue property_value = JS_GetProperty(ctx, this_obj, property_atom);
  JS_FreeAtom(ctx, property_atom);

  if(if_is_exception_then_free(ctx, property_value)) return;
  print_value(ctx, property_value);
  JS_FreeValue(ctx, property_value);
}


/**
 * 
 * e: 不管是不是exception, 都会被释放
 * returns 
 *  1: is exception
 *  0: is not exception 
 */
static int is_exception_free(JSContext *ctx, JSValue e)
{
  if(JS_IsException(e)) {
    print_exception_free(ctx, e);
    return 1;
  }else{
    JS_FreeValue(ctx, e);
    return 0;
  }
}

/**
 * 
 * e: 只有是exception, 才会被释放
 * returns 
 *  1: is exception
 *  0: is not exception 
 */
static int if_is_exception_then_free(JSContext *ctx, JSValue e)
{
  if(JS_IsException(e)) {
    print_exception_free(ctx, e);
    return 1;
  }else{
    return 0;
  }
}
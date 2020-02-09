#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libpq-fe.h>

#include <quickjs/quickjs.h>
#include <quickjs/quickjs-libc.h>

#define DEFAULT_JS_OBJECT_CLASS_ID 1

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
  const char* msg = JS_ToCString(ctx, e);
  printf("ERROR: %s\n", msg);
  JS_FreeCString(ctx, msg);
}

int main(int argc, char* argv[])
{
  JSRuntime *runtime = JS_NewRuntime();
    JSContext *context = JS_NewContext(runtime);
      JSValue grobal = JS_GetGlobalObject(context);

      js_std_add_helpers(context, 0, NULL);

        /* system modules */
        js_init_module_std(context, "std");
        js_init_module_os(context, "os");

      JS_SetPropertyStr(context, grobal, "pg_connect_db", JS_NewCFunction(context, js_PQconnectdb, NULL, 0));
      // load script, and run
      const char* script = ""
        "try{"
        "console.log('run js_postgres_example\\n');"
        "var conn = pg_connect_db('');"
        "var rs = conn.query('select name, category, setting, unit, short_desc from pg_settings');"
        "var rows = rs.getRowCount();"
        "var cols=rs.getColumnCount();"
        "console.log('rows='+rows+', cols='+cols+'\\n');"
        // "rs.print();"
        "for(let row=0;row<rows;row++){"
        "  for(let col=0;col<cols;col++){"
        "    print(row, col, rs.getValue(row, col));"
        "  }"
        "  print();"
        "}"
        "rs.close();"
        "conn.close();"
        "}catch(err){"
        "  console.log(err);"
        "}"
      ;
      JSValue rs = JS_Eval(context, script, strlen(script), "js_postgres_example", 0);
      if(JS_IsException(rs)) {
        JSValue exp = JS_GetException(context);
        if(JS_IsError(context, exp)){
          print_exception(context, exp);
          // error对象的属性
          // message
          // fileName
          // lineNumber
          // stack
        }
        JS_FreeValue(context, exp);
      }

  clean:
      JS_FreeValue(context, grobal);
    JS_FreeContext(context);
  JS_FreeRuntime(runtime);

  //JS_Eval(ctx, input, )

  return 0;
}
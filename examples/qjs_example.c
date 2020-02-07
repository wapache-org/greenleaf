#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include <quickjs/quickjs.h>
#include <quickjs/quickjs-libc.h>

typedef int BOOL;

#ifndef FALSE
enum {
    FALSE = 0,
    TRUE = 1,
};
#endif

static int qjs_example(char* path);

static JSValue js_loadScript(JSContext *ctx, const char *filename);

static uint8_t *load_file(JSContext *ctx, size_t *pbuf_len, const char *filename);


static void print_exception(JSContext *ctx, JSValue e);
static void print_exception_free(JSContext *ctx, JSValue e);
static int is_exception_free(JSContext *ctx, JSValue e);
static int if_is_exception_then_free(JSContext *ctx, JSValue e);
static void print_value(JSContext *ctx, JSValue e);
static void print_property(JSContext *ctx, JSValue this_obj, const char* property_name);


static JSValue js_eval(JSContext *ctx, const char*buf);
static JSValue js_eval_json(JSContext *ctx, const char*buf);

static const char* cs_engine_var_name = "engine";
static const char* cs_fn_name_get_template = "get_template";
static const char* cs_fn_name_render = "render";

int
main(int argc, char* argv[])
{
    char* path = argc>1 ? argv[1] : "template_engine.js";
    return qjs_example(path);
}

static JSValue sql_example(JSContext *context, const char* path, const char* json_data){

  uint8_t *buf;
  JSValue ret;
  size_t buf_len;
  
  if (!path)
      return JS_EXCEPTION;
  buf = load_file(context, &buf_len, path);
  if (!buf) {
      JS_ThrowReferenceError(context, "could not load '%s'", path);
      return JS_EXCEPTION;
  }

      JSValue grobal = JS_GetGlobalObject(context);
      JSValue tpl_value = JS_NewString(context, (char *)buf);

      {
        // var rs = render(template_str, data);
        JSValue data = js_eval_json(context, json_data);
        JSValue argv2[] = { tpl_value, data };
        JSAtom render_method_fn_name = JS_NewAtom(context, cs_fn_name_render);
        JSValue rs = JS_Invoke(context, grobal, render_method_fn_name, 2, argv2);
        JS_FreeAtom(context, render_method_fn_name);

        JS_FreeValue(context, data);
        if(if_is_exception_then_free(context, rs)) goto clean;

        // JS_ToString(context, rs);
        const char* rs_cstr = JS_ToCString(context, rs);
        printf("template: %s\nresult:%s\n", (char *)buf, rs_cstr);
        JS_FreeCString(context, rs_cstr);
        JS_FreeValue(context, rs);

      }

      //JS_ParseJSON(context, NULL,0,NULL);
clean:
      JS_FreeValue(context, tpl_value);
      JS_FreeValue(context, grobal);

    js_free(context, buf);
    return ret;

}


static int eval_buf(JSContext *ctx, const void *buf, int buf_len,
                    const char *filename, int eval_flags)
{
    JSValue val;
    int ret;

    if ((eval_flags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) {
        /* for the modules, we compile then run to be able to set
           import.meta */
        val = JS_Eval(ctx, buf, buf_len, filename,
                      eval_flags | JS_EVAL_FLAG_COMPILE_ONLY);
        if (!JS_IsException(val)) {
            js_module_set_import_meta(ctx, val, TRUE, TRUE);
            val = JS_EvalFunction(ctx, val);
        }
    } else {
        val = JS_Eval(ctx, buf, buf_len, filename, eval_flags);
    }
    if (JS_IsException(val)) {
        js_std_dump_error(ctx);
        ret = -1;
    } else {
        ret = 0;
    }
    JS_FreeValue(ctx, val);
    return ret;
}

static int qjs_example(char* path) {
  // var tpl = "...";
  const char* tpl = "the name is {{ d.name }}.";

  /// todo add fail check and free pointers
  JSRuntime *runtime = JS_NewRuntime();
    JSContext *context = JS_NewContext(runtime);
      JSValue grobal = JS_GetGlobalObject(context);
      JSValue tpl_value = JS_NewString(context, tpl);


        js_std_add_helpers(context, 0, NULL);

        /* system modules */
        js_init_module_std(context, "std");
        js_init_module_os(context, "os");

        /* make 'std' and 'os' visible to non module code */
        {
            const char *str = 
              "import * as std from 'std';\n"
              "import * as os from 'os';\n"
              "globalThis.std = std;\n"
              "globalThis.os = os;\n";
            if(eval_buf(context, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE)!=0){
              exit(EXIT_FAILURE);
            }
        }


      if(is_exception_free(context, js_loadScript(context, path))) goto clean;

      {
        JSAtom engineVarName = JS_NewAtom(context, cs_engine_var_name);
        JSValue engine = JS_GetProperty(context, grobal, engineVarName);
        JS_FreeAtom(context, engineVarName);

        if(if_is_exception_then_free(context, engine)) goto clean;
        JS_FreeValue(context, engine);
      }
      
      {
        // var template = get_template(tpl);
        JSAtom  fn_name = JS_NewAtom(context, cs_fn_name_get_template);
        JSValue template = JS_Invoke(context, grobal, fn_name, 1, &tpl_value);

        JS_FreeAtom(context, fn_name);
        if(if_is_exception_then_free(context, template)) goto clean;

        // var rs = template.render(data);
        JSValue data = JS_NewObject(context);
        //{
          JSAtom property_name = JS_NewAtom(context, "name");
          JSValue property_value = JS_NewString(context, "wapa666");

          JS_SetProperty(context, data, property_name, property_value);
          //print_value(context, data);
          //print_property(context, data, "name");

          //JS_FreeAtom(context, property_name); // set 到一个js_object里就不需要再手动
          //JS_FreeValue(context, property_value);
        //}

        JSAtom render_method_fn_name = JS_NewAtom(context, cs_fn_name_render);
        JSValue render_fn = JS_GetProperty(context, template, render_method_fn_name);
        JS_FreeAtom(context, render_method_fn_name);

        //print_value(context, render_fn);
        JSValue args[] = {data, JS_UNDEFINED};
        //JSValue rs = JS_Invoke(context, template, render_method_fn_name, 2, args);
        JSValue rs = JS_Call(context, render_fn, template, 2, args);
        //print_property(context, template, "cache");

        JS_FreeValue(context, data);
        JS_FreeValue(context, render_fn);
        JS_FreeValue(context, template);
        if(if_is_exception_then_free(context, rs)) goto clean;
        
        const char* rs_cstr = JS_ToCString(context, rs);
        printf("template: %s\nresult:%s\n", tpl, rs_cstr);
        JS_FreeCString(context, rs_cstr);
        JS_FreeValue(context, rs);

      }

      {
        // var rs = render(template_str, data);
        JSValue data = JS_NewObject(context);{
          // 因为set进去了, 所以不用手工释放, 那就没有必要定义变量了
          // JSAtom property_name = JS_NewAtom(context, "name");
          // JSValue property_value = JS_NewString(context, "wapa666");
          JS_SetProperty(context, data, JS_NewAtom(context, "name"), JS_NewString(context, "wapa666"));
        }
        JSValue argv2[] = { tpl_value, data };
        JSAtom render_method_fn_name = JS_NewAtom(context, cs_fn_name_render);
        JSValue rs = JS_Invoke(context, grobal, render_method_fn_name, 2, argv2);
        JS_FreeAtom(context, render_method_fn_name);

        JS_FreeValue(context, data);
        if(if_is_exception_then_free(context, rs)) goto clean;

        // JS_ToString(context, rs);
        const char* rs_cstr = JS_ToCString(context, rs);
        printf("template: %s\nresult:%s\n", tpl, rs_cstr);
        JS_FreeCString(context, rs_cstr);
        JS_FreeValue(context, rs);

      }

      sql_example(context, "../../templates/dashboard/sql/default/activity.sql", "{\"did\":null}");
      sql_example(context, "../../templates/dashboard/sql/default/activity.sql", "{\"did\":123}");

      //JS_ParseJSON(context, NULL,0,NULL);
clean:
      JS_FreeValue(context, tpl_value);
      JS_FreeValue(context, grobal);
    JS_FreeContext(context);
  JS_FreeRuntime(runtime);

  //JS_Eval(ctx, input, )

  return 0;
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

static JSValue js_eval(JSContext *ctx, const char*buf)
{
    return JS_Eval(ctx, buf, strlen(buf), NULL, JS_EVAL_TYPE_GLOBAL);
}
static JSValue js_eval_json(JSContext *ctx, const char*buf)
{
    return JS_ParseJSON(ctx, buf, strlen(buf), NULL);
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
        JS_FreeCString(ctx, filename);
        return JS_EXCEPTION;
    }
    ret = JS_Eval(ctx, (char *)buf, buf_len, filename,
                  JS_EVAL_TYPE_GLOBAL);
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
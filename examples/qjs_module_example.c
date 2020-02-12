#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#if defined(__APPLE__)
#include <malloc/malloc.h>
#elif defined(__linux__)
#include <malloc.h>
#endif

#include <quickjs/quickjs-libc.h>


#ifndef countof
#define countof(x) (sizeof(x) / sizeof((x)[0]))
#endif

typedef int BOOL;

#ifndef FALSE
enum {
    FALSE = 0,
    TRUE = 1,
};
#endif


extern int has_suffix(const char *str, const char *suffix);

/** 从buf[buf_len]中读取js代码, 相当于 eval(buf); */
static int eval_buf(
    JSContext *ctx,
    const void *buf,
    int buf_len,
    const char *filename, // 给buf一个文件名, 方便出错打印信息
    int eval_flags        // eval的一些选项和标记, JS_EVAL_TYPE_GLOBAL: 全局方式, JS_EVAL_TYPE_MODULE: js module方式
)
{
    JSValue val;
    int ret;

    if ((eval_flags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) { // 以ES6 module方式eval
        /* for the modules, we compile then run to be able to set import.meta */
        val = JS_Eval(ctx, buf, buf_len, filename, eval_flags | JS_EVAL_FLAG_COMPILE_ONLY);
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

// 读取文件的内容并且eval
static int eval_file(
    JSContext *ctx,
    const char *filename,
    int module // 小于零则通过文件后缀和文件内容来猜测是不是一个js module, ==0: 不是, >0: 是
)
{
    uint8_t *buf;
    int ret, eval_flags;
    size_t buf_len;

    buf = js_load_file(ctx, &buf_len, filename);
    if (!buf) {
        perror(filename);
        exit(1);
    }

    if (module < 0) {
        module = (has_suffix(filename, ".mjs") ||
                  JS_DetectModule((const char *)buf, buf_len));
    }
    if (module)
        eval_flags = JS_EVAL_TYPE_MODULE;
    else
        eval_flags = JS_EVAL_TYPE_GLOBAL;
    ret = eval_buf(ctx, buf, buf_len, filename, eval_flags);
    js_free(ctx, buf);
    return ret;
}

// ./qjs_module_example ../quickjs_modules/api_request_handler.js

int main(int argc, char **argv)
{
    JSRuntime *rt;
    JSContext *ctx;

    rt = JS_NewRuntime();

    if (!rt) {
        fprintf(stderr, "qjs: cannot allocate JS runtime\n");
        exit(2);
    }
    ctx = JS_NewContext(rt);
    if (!ctx) {
        fprintf(stderr, "qjs: cannot allocate JS context\n");
        exit(2);
    }

    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);

    js_std_add_helpers(ctx, 0, NULL);

    const char *filename = argv[1];
    if (eval_file(ctx, filename, -1))
        goto fail;

    //js_std_loop(ctx);


    js_std_free_handlers(rt);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);

    return 0;
 fail:
    js_std_free_handlers(rt);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return 1;
}

// 在js里import模块的路径问题
// import a from './abc.js' # .开头 表示 相对于当前js文件的路径
// import c from '/abc.js'  # /开头 表示 绝对路径
// import b from 'abc.js'   # 不是.和/开头 表示 相对于当前工作目录的路径

// typedef char *JSModuleNormalizeFunc(JSContext *ctx,
//                                     const char *module_base_name,
//                                     const char *module_name, void *opaque);
// typedef JSModuleDef *JSModuleLoaderFunc(JSContext *ctx,
//                                         const char *module_name, void *opaque);

// /* default module filename normalizer */
// static char *js_default_module_normalize_name(
//     JSContext *ctx,
//     const char *base_name,
//     const char *name
// ){
//     char *filename, // file name with normalized full path
//     *p; // file name with '/' prefix

//     const char *r;
//     int len; // file path length

//     if (name[0] != '.') {
//         /* if no initial dot, the module name is not modified */
//         return js_strdup(ctx, name);
//     }

//     p = strrchr(base_name, '/');
//     if (p)
//         len = p - base_name;
//     else
//         len = 0;

//     filename = js_malloc(ctx, len + strlen(name) + 1 + 1);  // 1: '/' , 1: '\0'
//     if (!filename)
//         return NULL;
//     memcpy(filename, base_name, len);
//     filename[len] = '\0';

//     /* we only normalize the leading '..' or '.' */
//     r = name;
//     for(;;) {
//         if (r[0] == '.' && r[1] == '/') {
//             r += 2;
//         } else if (r[0] == '.' && r[1] == '.' && r[2] == '/') {
//             /* remove the last path element of filename, except if "."
//                or ".." */
//             if (filename[0] == '\0')
//                 break;
//             p = strrchr(filename, '/');
//             if (!p)
//                 p = filename;
//             else
//                 p++;
//             if (!strcmp(p, ".") || !strcmp(p, ".."))
//                 break;
//             if (p > filename)
//                 p--;
//             *p = '\0';
//             r += 3;
//         } else {
//             break;
//         }
//     }
//     if (filename[0] != '\0')
//         strcat(filename, "/");
//     strcat(filename, r);
//     //    printf("normalize: %s %s -> %s\n", base_name, name, filename);
//     return filename;
// }

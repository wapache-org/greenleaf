#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <quickjs/quickjs.h>
#include <quickjs/quickjs-libc.h>

#include "mongoose.h"

// ////////////////////////////////////////////////////////////////////////////
// head decleare

#define countof(x) (sizeof(x) / sizeof((x)[0]))

#define JS_REQUEST_CLASS_NAME "Request"
static char* mg_request_get_header(struct http_message* hm, char* header);
static JSValue js_request_get_header(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);


// ////////////////////////////////////////////////////////////////////////////
/* #region module init */ 

JSModuleDef *js_init_module_mongoose(JSContext *ctx, const char *module_name)
{
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_module_init_func);
    if (!m)
        return NULL;

    
    // add exports
    JS_AddModuleExport(ctx, m, JS_REQUEST_CLASS_NAME);

    return m;
}

static int js_module_init_func(JSContext *ctx, JSModuleDef *m)
{
    // init classes
    js_request_class_init(ctx, m);
    // 

}

/* #endregion */
// ////////////////////////////////////////////////////////////////////////////
// #region Request class define

static JSClassID js_request_class_id;
static JSClassDef js_request_class = {
    JS_REQUEST_CLASS_NAME,
    .finalizer = js_request_finalizer,
}; 
static const JSCFunctionListEntry js_request_proto_funcs[] = {
    // property getter/setter
    JS_CGETSET_MAGIC_DEF("method", js_request_getter, js_request_setter, 0),
    JS_CGETSET_MAGIC_DEF("uri", js_request_getter, js_request_setter, 1),
    JS_CGETSET_MAGIC_DEF("path", js_request_getter, js_request_setter, 0),
    JS_CGETSET_MAGIC_DEF("query_string", js_request_getter, js_request_setter, 1),
    // methods
    JS_CFUNC_DEF("getHeader", 0, js_request_get_header),
};
static int js_request_class_init(JSContext *ctx, JSModuleDef *m)
{
    JSValue proto, class;
    
    /* create the Point class */
    JS_NewClassID(&js_request_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_request_class_id, &js_request_class);

    proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, proto, js_request_proto_funcs, countof(js_request_proto_funcs));
    JS_SetClassProto(ctx, js_request_class_id, proto);
    
    class = JS_NewCFunction2(ctx, js_request_ctor, JS_REQUEST_CLASS_NAME, 2, JS_CFUNC_constructor, 0);
    /* set proto.constructor and ctor.prototype */
    JS_SetConstructor(ctx, class, proto);
                      
    JS_SetModuleExport(ctx, m, JS_REQUEST_CLASS_NAME, class);
    return 0;
}

// ////////////////////////////////////////////////////////
// #region Request class implementment

typedef struct request {
    struct http_message* hm;
    char* method;
    char* uri;
    char* path;
    char* query_string;
    char* (*get_header)(struct http_message* hm, char* header);
} Request;

static JSValue js_request_ctor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv)
{
    Request* request;
    JSValue obj = JS_UNDEFINED;
    JSValue proto;
    
    request = js_mallocz(ctx, sizeof(*request));
    if (!request)
        return JS_EXCEPTION;
        
    /* using new_target to get the prototype is necessary when the
       class is extended. */
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto))
        goto fail;
    obj = JS_NewObjectProtoClass(ctx, proto, js_request_class_id);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))
        goto fail;
    
    JS_SetOpaque(obj, request);
    
    return obj;
 fail:
    js_free(ctx, request);
    JS_FreeValue(ctx, obj);
    return JS_EXCEPTION;
}

static void js_request_finalizer(JSRuntime *rt, JSValue val)
{
    struct request *s = JS_GetOpaque(val, js_request_class_id);
    /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
    js_free_rt(rt, s);
}

static JSValue js_request_getter(JSContext *ctx, JSValueConst this_val, int magic)
{
    Request* s = JS_GetOpaque2(ctx, this_val, js_request_class_id);
    if (!s)
        return JS_EXCEPTION;
    switch(magic){

    }
    return JS_UNDEFINED;
}

static JSValue js_request_setter(JSContext *ctx, JSValueConst this_val, JSValue val, int magic)
{
    int v;

    Request* s = JS_GetOpaque2(ctx, this_val, js_request_class_id);
    if (!s)
        return JS_EXCEPTION;

    if (JS_ToInt32(ctx, &v, val))
        return JS_EXCEPTION;

    switch(magic){

    }
    return JS_UNDEFINED;
}

// methods




// ////////////////////////////////////////////////////////////////////////////
// #region Request class define

static JSClassID js_request_class_id;
static JSClassDef js_request_class = {
    JS_REQUEST_CLASS_NAME,
    .finalizer = js_request_finalizer,
}; 
static const JSCFunctionListEntry js_request_proto_funcs[] = {
    // property getter/setter
    JS_CGETSET_MAGIC_DEF("method", js_request_getter, js_request_setter, 0),
    JS_CGETSET_MAGIC_DEF("uri", js_request_getter, js_request_setter, 1),
    JS_CGETSET_MAGIC_DEF("path", js_request_getter, js_request_setter, 0),
    JS_CGETSET_MAGIC_DEF("query_string", js_request_getter, js_request_setter, 1),
    // methods
    JS_CFUNC_DEF("getHeader", 0, js_request_get_header),
};
static int js_request_class_init(JSContext *ctx, JSModuleDef *m)
{
    JSValue proto, class;
    
    /* create the Point class */
    JS_NewClassID(&js_request_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_request_class_id, &js_request_class);

    proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, proto, js_request_proto_funcs, countof(js_request_proto_funcs));
    JS_SetClassProto(ctx, js_request_class_id, proto);
    
    class = JS_NewCFunction2(ctx, js_request_ctor, JS_REQUEST_CLASS_NAME, 2, JS_CFUNC_constructor, 0);
    /* set proto.constructor and ctor.prototype */
    JS_SetConstructor(ctx, class, proto);
                      
    JS_SetModuleExport(ctx, m, JS_REQUEST_CLASS_NAME, class);
    return 0;
}

// #endregion Request class implementment
// ////////////////////////////////////////////////////////
// #region http functions implementment

static JSValue http_send(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    return JS_EXCEPTION;
}

static void js_request_finalizer(JSRuntime *rt, JSValue val)
{
    struct request *s = JS_GetOpaque(val, js_request_class_id);
    /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
    js_free_rt(rt, s);
}

static JSValue js_request_getter(JSContext *ctx, JSValueConst this_val, int magic)
{
    Request* s = JS_GetOpaque2(ctx, this_val, js_request_class_id);
    if (!s)
        return JS_EXCEPTION;
    switch(magic){

    }
    return JS_UNDEFINED;
}

static JSValue js_request_setter(JSContext *ctx, JSValueConst this_val, JSValue val, int magic)
{
    int v;

    Request* s = JS_GetOpaque2(ctx, this_val, js_request_class_id);
    if (!s)
        return JS_EXCEPTION;

    if (JS_ToInt32(ctx, &v, val))
        return JS_EXCEPTION;

    switch(magic){

    }
    return JS_UNDEFINED;
}

// methods



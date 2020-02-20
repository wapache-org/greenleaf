/*
 * Copyright (c) 2020 wapache.org
 * All rights reserved
 */

//header for getopt
#include <unistd.h>
//header for getopt_long
#include <getopt.h>

// header for postgresql
#include "libpq/libpq-fe.h"

// header for quickjs
#include <quickjs/quickjs-libc.h>

// header for mongoose
#include "mongoose.h"

// ////////////////////////////////////////////////////////////////////////////
// #region declare area

// ////////////////////////////////////////////////////////
// #region mongoose declare area

static const char *s_http_port = "8000";

static const struct mg_str s_get_method = MG_MK_STR("GET");
static const struct mg_str s_put_method = MG_MK_STR("PUT");
static const struct mg_str s_delele_method = MG_MK_STR("DELETE");

static int s_sig_num = 0;
static struct mg_serve_http_opts s_http_server_opts;

static void event_handler(struct mg_connection *nc, int ev, void *ev_data);

// #endregion mongoose declare area
// ////////////////////////////////////////////////////////
// #region API declare area

static const struct mg_str api_prefix = MG_MK_STR("/api/");

static void handle_api_request(struct mg_connection *nc, struct http_message *hm);
static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix);
static int is_equal(const struct mg_str *s1, const struct mg_str *s2);

static char* s_api_request_handler_func = "handle_api_request";
static char* s_api_request_handle_file = "qjs_modules/api_request_handler.js";
static void qjs_handle_api_request(struct mg_connection *nc, struct http_message *hm);

// #endregion API declare area
// ////////////////////////////////////////////////////////
// #region sqlite declare area


static void *s_db_handle = NULL;
static const char *s_db_path = "sqlite.db";

// #endregion sqlite declare area
// ////////////////////////////////////////////////////////
// #region quickjs declare area

#define DEFAULT_JS_OBJECT_CLASS_ID 1
static JSRuntime *runtime;
static JSContext *context;
static JSValue grobal;

static int qjs_runtime_init();
static int qjs_runtime_free();

static int eval_buf(
    JSContext *ctx,
    const void *buf,
    int buf_len,
    const char *filename, // 给buf一个文件名, 方便出错打印信息
    int eval_flags        // eval的一些选项和标记, JS_EVAL_TYPE_GLOBAL: 全局方式, JS_EVAL_TYPE_MODULE: js module方式
);
static int eval_file(
    JSContext *ctx,
    const char *filename,
    int module // 小于零则通过文件后缀和文件内容来猜测是不是一个js module, ==0: 不是, >0: 是
);
static uint8_t *load_file(JSContext *ctx, size_t *pbuf_len, const char *filename);

static void print_exception(JSContext *ctx, JSValue e);
static void print_exception_free(JSContext *ctx, JSValue e);
static int is_exception_free(JSContext *ctx, JSValue e);
static int if_is_exception_then_free(JSContext *ctx, JSValue e);
static void print_value(JSContext *ctx, JSValue e, const char* prefix);
static void print_property(JSContext *ctx, JSValue this_obj, const char* property_name);

static int js_execute_script(const char* script_file);

// #endregion quickjs declare area
// ////////////////////////////////////////////////////////
// #region quickjs-postgres declare area

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

// #endregion quickjs-postgres declare area
// ////////////////////////////////////////////////////////
// #region mongoose-security declare area

static const char *s_login_url = "/api/user/login.json";
static const char *s_logout_url = "/api/user/logout.json";
static const char *s_login_user = "username";
static const char *s_login_pass = "password";

static int send_cookie_auth_request(struct mg_connection *nc, char* message);

/*
 * If requested via GET, serves the login page.
 * If requested via POST (form submission), checks password and logs user in.
 */
static void login_handler(struct mg_connection *nc, int ev, void *p);


/*
 * Logs the user out.
 * Removes cookie and any associated session state.
 */
static void logout_handler(struct mg_connection *nc, int ev, void *p);

/*
 * Password check function.
 * In our example all users have password "password".
 */
static int check_pass(const char *user, const char *pass);


static int get_user_htpasswd(struct mg_str username, struct mg_str auth_domain, char* out_ha1);


// #endregion mongoose-security declare area
// ////////////////////////////////////////////////////////
// #region mongoose-session declare area

/* This is the name of the cookie carrying the session ID. */
#define SESSION_COOKIE_NAME "mgs"
/* In our example sessions are destroyed after 30 seconds of inactivity. */
#define SESSION_TTL 30.0
#define SESSION_CHECK_INTERVAL 5.0

/* Session information structure. */
struct session {
  /* Session ID. Must be unique and hard to guess. */
  uint64_t id;
  /*
   * Time when the session was created and time of last activity.
   * Used to clean up stale sessions.
   */
  double created;
  double last_used; /* Time when the session was last active. */

  /* User name this session is associated with. */
  char *user;
  /* Some state associated with user's session. */
  int lucky_number;
};

/*
 * This example uses a simple in-memory storage for just 10 sessions.
 * A real-world implementation would use persistent storage of some sort.
 */
#define NUM_SESSIONS 10
struct session s_sessions[NUM_SESSIONS];


/*
 * Creates a new session for the user.
 */
static struct session *create_session(const char *user, const struct http_message *hm);

/*
 * Destroys the session state.
 */
static void destroy_session(struct session *s);

/*
 * Parses the session cookie and returns a pointer to the session struct or NULL if not found.
 */
static struct session *get_session(struct http_message *hm);


static int get_session_id(struct http_message *hm, char *ssid, size_t len);


/* Cleans up sessions that have been idle for too long. */
void check_sessions(void);


// #endregion mongoose-session declare area
// ////////////////////////////////////////////////////////
// misc declare area


// #endregion postgres declare area
// ////////////////////////////////////////////////////////
// #region postgres declare area


// #endregion postgres declare area
// ////////////////////////////////////////////////////////
// misc declare area

static void usage(int argc, char* argv[]);
static int parse_options(struct mg_serve_http_opts* opts, int argc, char* argv[]);

static void signal_handler(int sig_num);


// #endregion declare area
// ////////////////////////////////////////////////////////////////////////////
// #region implement area

// 短选项字符串, 一个字母表示一个短参数, 如果字母后带有冒号, 表示这个参数必须带有参数
// 建议按字母顺序编写
static char* short_opts = "a:d:e:hlp:q:r:";
// 长选项字符串, 
// {长选项名字, 0:没有参数|1:有参数|2:参数可选, flags, 短选项名字}
// 建议按长选项字母顺序编写
static const struct option long_options[] = {
		{"auth-domain",1,NULL,'a'},
		{"database",1,NULL,'d'},
		{"enable-directory-listing",0,NULL,'l'},
		{"execute",1,NULL,'e'},
		{"help",0,NULL,'h'},
		{"port",1,NULL,'p'},
		{"root",1,NULL,'r'},
		{"qjs-api-router",1,NULL,'q'}
};
// 打印选项说明
static void usage(int argc, char* argv[])
{
  printf("Usages: \n");
  printf("    %s -e qjs-modules/hello.js\n", argv[0]);
  printf("    %s -p 8080 -r static\n", argv[0]);
  printf("Options:\n");
  printf("    [-%s, --%s]     %s\n", "h","help","print this message");
  printf("    [-%s, --%s]     %s\n", "a","auth-domain","the domain parameter of http digest");
  printf("    [-%s, --%s]     %s\n", "d","database","the database file path");
  printf("    [-%s, --%s]     %s\n", "e","execute","execute script");
  printf("    [-%s, --%s]     %s\n", "p","poot","web server bingding port, default is 8000.");
  printf("    [-%s, --%s]     %s\n", "q","qjs-api-router","web server api request route file, default is `qjs_modules/api_request_handler.js`.");
  printf("    [-%s, --%s]     %s\n", "r","root","web server root directory, default is `static`.");
  printf("    [-%s, --%s]     %s\n", "l","enable-directory-listing","if cannot find index file, list directory files, default is no.");
}

/**
 * 
 */
int main(int argc, char *argv[]) 
{

    if(0 != parse_options(&s_http_server_opts, argc,argv)){
      exit(EXIT_FAILURE);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Open database */
    // if ((s_db_handle = db_open(s_db_path)) == NULL) {
    //     fprintf(stderr, "Cannot open DB [%s]\n", s_db_path);
    //     exit(EXIT_FAILURE);
    // }

    /* Open listening socket */
    struct mg_mgr mgr;
    mg_mgr_init(&mgr, NULL);

    struct mg_connection *nc = mg_bind(&mgr, s_http_port, event_handler);
    mg_set_protocol_http_websocket(nc);
    mg_register_http_endpoint(nc, s_login_url, login_handler);
    mg_register_http_endpoint(nc, s_logout_url, logout_handler);
    mg_set_timer(nc, mg_time() + SESSION_CHECK_INTERVAL);

    // 初始化js runtime
    qjs_runtime_init();
    
    /* Run event loop until signal is received */
    printf("Starting server on port %s\n", s_http_port);
    while (s_sig_num == 0) {
        mg_mgr_poll(&mgr, 1000);
    }

    /* Cleanup */
    mg_mgr_free(&mgr);
    // db_close(&s_db_handle);

    qjs_runtime_free();

    printf("Exiting on signal %d\n", s_sig_num);

    return 0;
}

// 解析选项
static int parse_options(struct mg_serve_http_opts* opts, int argc, char* argv[])
{
  // 先设置默认值
  opts->document_root = "html";
  opts->enable_directory_listing = "no";
  opts->auth_domain = "localhost";
  opts->get_user_htpasswd_fn = get_user_htpasswd;

  // 如果只有短选项, 用getopt就够了
  // int code = getopt(argc, argv, short_opts);

  // 解析长选项和短选项
  int opt = 0;
  while((opt=getopt_long(argc,argv,short_opts,long_options,NULL))!=-1){
    switch (opt)
    {
    case 'h':
      usage(argc, argv);
      exit(EXIT_SUCCESS);
      break;
    case 'a':
      opts->auth_domain = optarg;
      break;
    case 'd':
      s_db_path = optarg;
      break;
    case 'e':
      return js_execute_script(optarg);
    case 'r':
      opts->document_root = optarg;
      break;
    case 'p':
      s_http_port = optarg;
      break;
    case 'q':
      s_api_request_handle_file = optarg;
      break;
    case 'l':
      opts->enable_directory_listing = "yes";
      break;
    
    default:
      usage(argc, argv);
      exit(EXIT_FAILURE);
      break;
    }
    return 0;
  }
}

static void signal_handler(int sig_num) 
{
    signal(sig_num, signal_handler);
    s_sig_num = sig_num;
}

static void event_handler(struct mg_connection *nc, int ev, void *ev_data) 
{
    struct http_message *hm = (struct http_message *) ev_data;

    switch (ev) {
    case MG_EV_HTTP_REQUEST:
        if (has_prefix(&hm->uri, &api_prefix)) 
        {
            struct mg_str api_menu = MG_MK_STR("/api/menu.json");
            if(is_equal(&hm->uri, &api_menu)){
                mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
                break;
            }

            int len;
            char ssid[32];
            if(len=get_session_id(hm, ssid, sizeof(ssid))){
                // len == strlen(ssid);
                struct session *s = get_session(hm);
                if(s == NULL){
                    send_cookie_auth_request(nc, "session expired.");
                    break;
                }
            }else if(!mg_http_custom_is_authorized(
                hm, s_http_server_opts.auth_domain, s_http_server_opts.get_user_htpasswd_fn
            )){
                mg_http_send_digest_auth_request(nc, s_http_server_opts.auth_domain);
                break;
            }

            handle_api_request(nc, hm);
        } else {
            mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
        }
        break;
    }
}

// ////////////////////////////////////////////////////////
// #region API implement area

static void handle_api_request(struct mg_connection *nc, struct http_message *hm)
{

    qjs_handle_api_request(nc,hm);

    // // 需要改成用hashmap
    // char* path = (char*)(hm->uri.p);
    // path = path+api_prefix.len-1;
    // int length = hm->uri.len-api_prefix.len-4; // +1-strlen(".json")
    // if (strncmp(path, "/services/host", length)==0)
    // {
    //     // 从数据库读取节点的信息
    //     // 然后连接到host, 获取更进一步的信息
    // }
    

    // mg_serve_http(nc, hm, s_http_server_opts);
}


static void qjs_handle_api_request(struct mg_connection *nc, struct http_message *hm)
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

        // hm->body
        char body[hm->body.len+1];
        memset(body, 0, sizeof(body));
        memcpy(body, hm->body.p, hm->body.len);

        JS_SetPropertyStr(context, request, "method", JS_NewString(context, method));
        JS_SetPropertyStr(context, request, "uri", JS_NewString(context, uri));
        JS_SetPropertyStr(context, request, "path", JS_NewString(context, path));
        JS_SetPropertyStr(context, request, "query_string", JS_NewString(context, query_string));
        JS_SetPropertyStr(context, request, "body", JS_NewString(context, body));
        
        // mg_get_http_header
        // mg_get_http_var

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
        JSAtom fn_name = JS_NewAtom(context, s_api_request_handler_func);
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


// #endregion API implement area
// ////////////////////////////////////////////////////////
// #region mongoose-security implement area

static int send_cookie_auth_request(struct mg_connection *nc, char* message)
{
    mg_printf(nc, "HTTP/1.0 403 Unauthorized\r\n\r\n%s\r\n", message == NULL ? "Unauthorized." : message);
    return 1;
}

static void login_handler(struct mg_connection *nc, int ev, void *p) 
{
  switch (ev) {
    case MG_EV_HTTP_REQUEST: {
        /* Perform password check. */
        char user[50], pass[50];
        int ul,pl;
      struct http_message *hm = (struct http_message *) p;
      if (mg_vcmp(&hm->method, "POST") != 0) {
        ul = mg_get_http_var(&hm->query_string, s_login_user, user, sizeof(user));
        pl = mg_get_http_var(&hm->query_string, s_login_pass, pass, sizeof(pass));
      } else {
        ul = mg_get_http_var(&hm->body, s_login_user, user, sizeof(user));
        pl = mg_get_http_var(&hm->body, s_login_pass, pass, sizeof(pass));
      }

      if (ul > 0 && pl > 0) {
        if (check_pass(user, pass)) {
          struct session *s = create_session(user, hm);
          mg_printf(nc, 
              "HTTP/1.0 200 OK\r\n"
              "Set-Cookie: %s=%" INT64_X_FMT "; path=/\r\n"
              "Content-Type: application/json;charset=utf-8\r\n"
              "\r\n"
              "{\"code\": 0,\"msg\": \"登入成功\",\"data\": {\"access_token\":\"%" INT64_X_FMT "\"}}\r\n",
              SESSION_COOKIE_NAME,
              s->id,
              s->id
          );
          fprintf(stderr, "%s logged in, sid %" INT64_X_FMT "\n", s->user, s->id);
        } else {
          mg_printf(nc, "HTTP/1.0 403 Unauthorized\r\n\r\nWrong password.\r\n");
        }
      } else {
        mg_printf(nc, "HTTP/1.0 400 Bad Request\r\n\r\nuser, pass required.\r\n");
      }
      nc->flags |= MG_F_SEND_AND_CLOSE;
    }
  }
}

static void logout_handler(struct mg_connection *nc, int ev, void *p) 
{
  switch (ev) {
    case MG_EV_HTTP_REQUEST: {
      struct http_message *hm = (struct http_message *) p;
      char shead[100];
      snprintf(shead, sizeof(shead), "Set-Cookie: %s=", SESSION_COOKIE_NAME);
      mg_http_send_redirect(nc, 302, mg_mk_str("/"), mg_mk_str(shead));
      struct session *s = get_session(hm);
      if (s != NULL) {
        fprintf(stderr, "%s logged out, session %" INT64_X_FMT " destroyed\n",
                s->user, s->id);
        destroy_session(s);
      }
      nc->flags |= MG_F_SEND_AND_CLOSE;
    }
  }
}

static int check_pass(const char *user, const char *pass) 
{
  (void) user;
  if((strcmp(pass, "admin") == 0)){
      return 1;
  }else{
    char ha1[128]; 
    if(get_user_htpasswd(mg_mk_str(user), mg_mk_str(s_http_server_opts.auth_domain), ha1) && (strcmp(pass, ha1) == 0)){
        return 1;
    }
  }
  return 0;
}

static int get_user_htpasswd(struct mg_str username, struct mg_str auth_domain, char* out_ha1)
{
    // out_ha1 is char[128]
    // username/password: admin/admin
    strcpy(out_ha1,"609e3552947b5949c2451a072a2963e1");
    return 1; // found
}

// #endregion mongoose-security implement area
// ////////////////////////////////////////////////////////
// #region mongoose-session implement area

static struct session *create_session(const char *user, const struct http_message *hm) 
{
  /* Find first available slot or use the oldest one. */
  struct session *s = NULL;
  struct session *oldest_s = s_sessions;
  int i;
  for (i = 0; i < NUM_SESSIONS; i++) {
    if (s_sessions[i].id == 0) {
      s = &s_sessions[i];
      break;
    }
    if (s_sessions[i].last_used < oldest_s->last_used) {
      oldest_s = &s_sessions[i];
    }
  }
  if (s == NULL) {
    destroy_session(oldest_s);
    printf("Evicted %" INT64_X_FMT "/%s\n", oldest_s->id, oldest_s->user);
    s = oldest_s;
  }
  /* Initialize new session. */
  s->created = s->last_used = mg_time();
  s->user = strdup(user);
  s->lucky_number = rand();
  /* Create an ID by putting various volatiles into a pot and stirring. */
  cs_sha1_ctx ctx;
  cs_sha1_init(&ctx);
  cs_sha1_update(&ctx, (const unsigned char *) hm->message.p, hm->message.len);
  cs_sha1_update(&ctx, (const unsigned char *) s, sizeof(*s));
  unsigned char digest[20];
  cs_sha1_final(digest, &ctx);
  s->id = *((uint64_t *) digest);
  return s;
}

static void destroy_session(struct session *s) 
{
  free(s->user);
  memset(s, 0, sizeof(*s));
}

static struct session *get_session(struct http_message *hm) 
{
  char ssid_buf[21];
  char *ssid = ssid_buf;
  struct session *ret = NULL;
  struct mg_str *cookie_header = mg_get_http_header(hm, "cookie");
  if (cookie_header == NULL) goto clean;
  if (!mg_http_parse_header2(
    cookie_header, SESSION_COOKIE_NAME, &ssid, sizeof(ssid_buf)
  )) {
    goto clean;
  }
  uint64_t sid = strtoull(ssid, NULL, 16);
  int i;
  for (i = 0; i < NUM_SESSIONS; i++) {
    if (s_sessions[i].id == sid) {
      s_sessions[i].last_used = mg_time();
      ret = &s_sessions[i];
      goto clean;
    }
  }

  clean:
  if (ssid != ssid_buf) {
    free(ssid);
  }
  return ret;
}

static int get_session_id(struct http_message *hm, char *ssid, size_t len) 
{
  struct mg_str *cookie_header = mg_get_http_header(hm, "cookie");
  if (cookie_header == NULL) {
      return 0;
  } else {
      return mg_http_parse_header2(cookie_header, SESSION_COOKIE_NAME, &ssid, len);
  }
}

void check_sessions(void) 
{
  double threshold = mg_time() - SESSION_TTL;
  int i;
  for (i = 0; i < NUM_SESSIONS; i++) {
    struct session *s = &s_sessions[i];
    if (s->id != 0 && s->last_used < threshold) {
      fprintf(stderr, "Session %" INT64_X_FMT " (%s) closed due to idleness.\n",
              s->id, s->user);
      destroy_session(s);
    }
  }
}

// #endregion mongoose-session implement area
// ////////////////////////////////////////////////////////
// #region quickjs implement area

static int qjs_runtime_init() 
{

  runtime = JS_NewRuntime();
  if (!runtime) {
      fprintf(stderr, "qjs: cannot allocate JS runtime\n");
      exit(2);
  }
  context = JS_NewContext(runtime);
  if (!context) {
      fprintf(stderr, "qjs: cannot allocate JS context\n");
      exit(2);
  }
  grobal = JS_GetGlobalObject(context);

  /* loader for ES6 modules */
  JS_SetModuleLoaderFunc(runtime, NULL, js_module_loader, NULL);

  // build in functions and modules
  js_std_add_helpers(context, 0, NULL);
  js_init_module_std(context, "std");
  js_init_module_os(context, "os");


  // /* make 'std' and 'os' visible to non module code */
  // const char *str = "import * as std from 'std';\n"
  //     "import * as os from 'os';\n"
  //     "globalThis.std = std;\n"
  //     "globalThis.os = os;\n";
  // eval_buf(context, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE);
  // 
  // if (eval_file(context, s_api_request_handle_file, 1)){
  //   qjs_runtime_free();
  //   exit(EXIT_FAILURE);
  // }
  
  /* make 'std' and 'os' visible to non module code */
  const char *str2 = "import %s from '%s';\n""globalThis.%s = %s;\n";
  char *str3 = (char *) malloc(strlen(str2)-8 + strlen(s_api_request_handle_file)+ (strlen(s_api_request_handler_func)*4 ));
  sprintf(str3, str2, s_api_request_handler_func, s_api_request_handle_file, s_api_request_handler_func, s_api_request_handler_func);
  eval_buf(context, str3, strlen(str3), "<input>", JS_EVAL_TYPE_MODULE);
  free(str3);

  JS_SetPropertyStr(context, grobal, "pg_connect_db", JS_NewCFunction(context, js_PQconnectdb, NULL, 0));

  return 0;
}


static int qjs_runtime_free() 
{
  JS_FreeValue(context, grobal);
  js_std_free_handlers(runtime);
  JS_FreeContext(context);
  JS_FreeRuntime(runtime);
  return 0;
}


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

static void JS_DumpErrorObject(JSContext *ctx, JSValue ex, const char* prefix) {

  JSValue message = JS_GetPropertyStr(context, ex, "message");
  print_value(context, message, "ERROR: ");
  JS_FreeValue(context, message);

  JSValue stack = JS_GetPropertyStr(context, ex, "stack");
  print_value(context, stack, "STACK: \n");
  JS_FreeValue(context, stack);

  // JSValue json = JS_JSONStringify(context, ex, JS_UNDEFINED, JS_UNDEFINED);
  // print_value(context, json, prefix);
  // JS_FreeValue(context, json);
}

static void print_exception(JSContext *ctx, JSValue e)
{
  assert(JS_IsException(e));

  // const char* msg = JS_ToCString(ctx, e);
  // printf("ERROR: %s\n", msg);
  // JS_FreeCString(ctx, msg);
  
  // Error { message: 1"expecting '('", fileName: 1"quickjs_modules/api_request_handler.js", lineNumber: 2, stack: 1"    at quickjs_modules/api_request_handler.js:2\n" }
  JSValue ex = JS_GetException(context);
  if(JS_IsError(context, ex)){
    JS_DumpErrorObject(context, ex, "ERROR: ");
  }else{
    print_value(context, ex, "ERROR: ");
  }
  JS_FreeValue(context, ex);
}

static void print_exception_free(JSContext *ctx, JSValue e)
{
  print_exception(ctx, e);
  JS_FreeValue(ctx, e);
}

static void print_value(JSContext *ctx, JSValue e, const char* prefix)
{
  const char* msg = JS_ToCString(ctx, e);
  printf("%s%s\n", prefix, msg);
  JS_FreeCString(ctx, msg);
}

static void print_property(JSContext *ctx, JSValue this_obj, const char* property_name)
{
  JSAtom property_atom = JS_NewAtom(ctx, property_name);
  JSValue property_value = JS_GetProperty(ctx, this_obj, property_atom);
  JS_FreeAtom(ctx, property_atom);

  if(if_is_exception_then_free(ctx, property_value)) return;
  print_value(ctx, property_value, "DUMP: ");
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


// #endregion quickjs implement area
// ////////////////////////////////////////////////////////
// #region quickjs-postgres implement area

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

static int js_execute_script(const char* script_file){

    JSRuntime *rt;
    JSContext *ctx;

    rt = JS_NewRuntime();
    if (!rt) {
        fprintf(stderr, "qjs: cannot allocate JS runtime\n");
        return 3;
    }

    ctx = JS_NewContext(rt);
    if (!ctx) {
        fprintf(stderr, "qjs: cannot allocate JS context\n");
        return 3;
    }

    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);

    js_std_add_helpers(ctx, 0, NULL);

    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");

    JSValue g = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, g, "pg_connect_db", JS_NewCFunction(ctx, js_PQconnectdb, NULL, 0));
    JS_FreeValue(ctx, g);

    if (eval_file(ctx, script_file, -1))
        goto fail;

    js_std_free_handlers(rt);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);

    return 1;
 fail:
    js_std_free_handlers(rt);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return 2;
}
// #endregion quickjs-postgres implement area
// ////////////////////////////////////////////////////////
// #region misc implement area

// #endregion misc implement area
// ////////////////////////////////////////////////////////////////////////////

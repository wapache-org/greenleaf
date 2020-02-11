/*
 * Copyright (c) 2020 wapache.org
 * All rights reserved
 */

//header for getopt
#include <unistd.h>
//header for getopt_long
#include <getopt.h>

// header for postgresql
#include <libpq-fe.h>

// header for quickjs
#include <quickjs/quickjs.h>
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

static char* s_api_request_handle_file = "quickjs_modules/api_request_handler.js";
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

static JSValue js_loadScript(JSContext *ctx, const char *filename);
static uint8_t *load_file(JSContext *ctx, size_t *pbuf_len, const char *filename);
static void print_exception(JSContext *ctx, JSValue e);
static void print_exception_free(JSContext *ctx, JSValue e);
static int is_exception_free(JSContext *ctx, JSValue e);
static int if_is_exception_then_free(JSContext *ctx, JSValue e);
static void print_value(JSContext *ctx, JSValue e);
static void print_property(JSContext *ctx, JSValue this_obj, const char* property_name);

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
static void parse_options(struct mg_serve_http_opts* opts, int argc, char* argv[]);

static void signal_handler(int sig_num);


// #endregion declare area
// ////////////////////////////////////////////////////////////////////////////
// #region implement area

// 短选项字符串, 一个字母表示一个短参数, 如果字母后带有冒号, 表示这个参数必须带有参数
// 建议按字母顺序编写
static char* short_opts = "a:d:hlp:q:r:";
// 长选项字符串, 
// {长选项名字, 0:没有参数|1:有参数|2:参数可选, flags, 短选项名字}
// 建议按长选项字母顺序编写
static const struct option long_options[] = {
		{"auth-domain",1,NULL,'a'},
		{"database",1,NULL,'d'},
		{"enable-directory-listing",0,NULL,'l'},
		{"help",0,NULL,'h'},
		{"port",1,NULL,'p'},
		{"root",1,NULL,'r'},
		{"qjs-api-router",1,NULL,'q'}
};
// 打印选项说明
static void usage(int argc, char* argv[])
{
  printf("Usages: \n");
  printf("    %s -p 8080 -r html\n", argv[0]);
  printf("Options:\n");
  printf("    [-%s, --%s]     %s\n", "h","help","print this message");
  printf("    [-%s, --%s]     %s\n", "a","auth-domain","the domain parameter of http digest");
  printf("    [-%s, --%s]     %s\n", "d","database","the database file path");
  printf("    [-%s, --%s]     %s\n", "p","poot","web server bingding port, default is 8000.");
  printf("    [-%s, --%s]     %s\n", "q","qjs-api-router","web server api request route file, default is `quickjs_modules/api_request_handler.js`.");
  printf("    [-%s, --%s]     %s\n", "r","root","web server root directory, default is `html`.");
  printf("    [-%s, --%s]     %s\n", "l","enable-directory-listing","if cannot find index file, list directory files, default is no.");
}

/**
 * 
 */
int main(int argc, char *argv[]) 
{

    parse_options(&s_http_server_opts, argc,argv);

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
static void parse_options(struct mg_serve_http_opts* opts, int argc, char* argv[])
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
        JSAtom fn_name = JS_NewAtom(context, "handle_api_request");
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
  context = JS_NewContext(runtime);
  grobal = JS_GetGlobalObject(context);

  js_std_add_helpers(context, 0, NULL);

  js_loadScript(context, s_api_request_handle_file);

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

// #endregion quickjs-postgres implement area
// ////////////////////////////////////////////////////////
// #region misc implement area

// #endregion misc implement area
// ////////////////////////////////////////////////////////////////////////////

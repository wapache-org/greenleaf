/*
 * Copyright (c) 2020 wapache.org
 * All rights reserved
 */

// ////////////////////////////////////////////////////////////////////////////
// #region include area

//header for getopt
#include <unistd.h>
//header for getopt_long
#include <getopt.h>

// header for postgresql
#include "libpq-fe.h"

// header for quickjs
#include <quickjs/quickjs-libc.h>

#include "libssh/callbacks.h"
#include "libssh/libssh.h"
#include "libssh/sftp.h"

// header for mongoose
#include "mongoose.h"
#include "ssh/ssh_common.h"

// #endregion include area
// ////////////////////////////////////////////////////////////////////////////
// #region declare area

// ////////////////////////////////////////////////////////
// #region main declare area

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

// 解析选项
static void parse_options(int argc, char* argv[]);

// 
static void signal_handler(int sig_num);

// #endregion main declare area
// ////////////////////////////////////////////////////////
// #region mongoose declare area

static const struct mg_str s_get_method = MG_MK_STR("GET");
static const struct mg_str s_put_method = MG_MK_STR("PUT");
static const struct mg_str s_delele_method = MG_MK_STR("DELETE");

static void event_handler(struct mg_connection *nc, int ev, void *ev_data);

static int mg_str_has_prefix(const struct mg_str *uri, const struct mg_str *prefix);
static int mg_str_is_equal(const struct mg_str *s1, const struct mg_str *s2);

// #endregion mongoose declare area
// ////////////////////////////////////////////////////////
// #region mongoose-websocket declare area

static int is_websocket(const struct mg_connection *nc) ;

// #endregion mongoose-websocket declare area
// ////////////////////////////////////////////////////////
// #region mongoose-session declare area

/* This is the name of the cookie carrying the session ID. */
#define SESSION_COOKIE_NAME "GL"

/* In our example sessions are destroyed after 30 seconds of inactivity. */
// TODO 需要改成从配置文件或数据库读取
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
// TODO 需要改为保存到数据库或文件, 无大小限制
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
// #region mongoose-security declare area

static const char *s_login_url = "/api/user/login.json";
static const char *s_logout_url = "/api/user/logout.json";
static const char *s_login_user = "username";
static const char *s_login_pass = "password";

// 检查是否已认证
static int check_authentication(struct mg_connection *nc, struct http_message *hm);

// 发送"给予cookie的表单登录"要求给客户端
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

// 根据用户名获取用户密码, 返回的密码有可能是明文或密文
static int get_user_htpasswd(struct mg_str username, struct mg_str auth_domain, char* out_password);

// #endregion mongoose-security declare area
// ////////////////////////////////////////////////////////
// #region mongoose-api declare area

static const struct mg_str api_prefix = MG_MK_STR("/api/");
static void handle_api_request(struct mg_connection *nc, struct http_message *hm);

static void qjs_handle_api_request(JSContext* context, struct mg_connection *nc, struct http_message *hm);

// #endregion-api declare area
// ////////////////////////////////////////////////////////
// #region websocket~ssh declare area
#define WS_SSH_READ_BUF_SIZE 1024

// This info is passed by the worker thread to mg_broadcast
struct ws_ssh_context 
{
  struct mg_connection *nc;
  int status; // 0: inited, 1: ssh connected, -1: ssh disconnected

  char host[64];
  char port[6];
  char user[32];
  char password[64];

  ssh_session session;
  ssh_channel channel;

  void* ws_read_buf;
  int ws_read_buf_len;
  int ws_read_buf_size;

  void* ssh_read_buf;
  int ssh_read_buf_len;
  int ssh_read_buf_size;
};

static struct ws_ssh_context* new_ws_ssh_context(struct mg_connection *nc)
{
  // printf("invoke new_ws_ssh_context\n");
  struct ws_ssh_context* context = malloc(sizeof(struct ws_ssh_context));
  memset(context, 0, sizeof(struct ws_ssh_context));

  context->nc = nc;
  
  context->status = 0;
  memset(context->host, 0, sizeof(context->host));
  memset(context->port, 0, sizeof(context->port));
  memset(context->user, 0, sizeof(context->user));
  memset(context->password, 0, sizeof(context->password));
  context->session = ssh_new();
  context->channel = NULL;

  context->ws_read_buf_size = WS_SSH_READ_BUF_SIZE;
  context->ws_read_buf = malloc(context->ws_read_buf_size);
  context->ws_read_buf_len = 0;

  context->ssh_read_buf_size = WS_SSH_READ_BUF_SIZE*100;
  context->ssh_read_buf = malloc(context->ssh_read_buf_size);
  context->ssh_read_buf_len = 0;
    
  return context;
}

static void free_ws_ssh_context(struct ws_ssh_context *context)
{
  printf("invoke free_ws_ssh_context\r\n");
  if(context==NULL) return;

  context->status = 2;

  if(context->channel!=NULL && ssh_channel_is_open(context->channel)){
    ssh_channel_close(context->channel);
    context->channel = NULL;
  }

  if(context->session!=NULL){
    ssh_disconnect(context->session);
    ssh_free(context->session);
    context->session = NULL;
  }

  free(context->ws_read_buf);
  free(context->ssh_read_buf);

  free(context);
}


static int ws_ssh_connect(struct ws_ssh_context *context);
static int ssh_authenticate(ssh_session session, char* password);

extern void ssh_print_supported_auth_methods(int method_mark);
extern void ssh_print_error(ssh_session session);

// #endregion postgres declare area
// ////////////////////////////////////////////////////////
// #region quickjs declare area

#define DEFAULT_JS_OBJECT_CLASS_ID 1
static JSRuntime *runtime;
static JSContext *context;
// static JSValue grobal;

static int qjs_runtime_init();
static int qjs_runtime_free();

static JSContext* qjs_context_init();
static int qjs_context_free(JSContext* context);

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

static JSValue js_PQfnumber(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_PQfname(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static JSValue js_PQftype(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_PQfsize(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static JSValue js_PQgetvalue(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_PQclear(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

// #endregion quickjs-postgres declare area
// ////////////////////////////////////////////////////////
// #region sqlite declare area

static void *s_db_handle = NULL;
static const char *s_db_path = "sqlite.db";

// #endregion sqlite declare area
// ////////////////////////////////////////////////////////
// #region postgres declare area

// #endregion postgres declare area
// ////////////////////////////////////////////////////////
// #region debug area

#include <stdio.h>
#include <execinfo.h>

enum Constexpr { MAX_SIZE = 1024 };

char** get_call_stack() {
    void *array[MAX_SIZE];
    size_t size = backtrace(array, MAX_SIZE);
    char ** strings = backtrace_symbols(array, size);
    return strings;
}

void print_call_stack(void) {
    char **strings;
    size_t i, size;
    enum Constexpr { MAX_SIZE = 1024 };
    void *array[MAX_SIZE];
    size = backtrace(array, MAX_SIZE);
    strings = backtrace_symbols(array, size);
    for (i = 0; i < size; i++)
        printf("%s\r\n", strings[i]);
    puts("");
    free(strings);
}

// #endregion debug area
// ////////////////////////////////////////////////////////
// #region misc declare area
// #endregion misc declare area
// ////////////////////////////////////////////////////////
// #endregion declare area
// ////////////////////////////////////////////////////////////////////////////
// #region implement area
// ////////////////////////////////////////////////////////
// #region options implement area
struct main_options {

  struct mg_mgr mg_manager;
  struct mg_serve_http_opts mg_options;
  char * mg_http_port;

  struct mg_connection* mg_acceptor;
  
  char * sqlite_path;
  void * sqlite_handle;

  char * api_handle_file_path;
  char * api_handle_function;

  char * execute_script_path;

  int signal;

};

static struct main_options s_options;

static void parse_options(int argc, char* argv[])
{
  s_options.mg_http_port = "8000";
  s_options.sqlite_path = "greenleaf.db";
  s_options.api_handle_file_path = "qjs_modules/api_request_handler.js";
  s_options.api_handle_function = "handle_api_request";

  s_options.execute_script_path = NULL;

  struct mg_serve_http_opts* opts = &s_options.mg_options;

  // 先设置默认值
  opts->document_root = "static";
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
      s_options.sqlite_path = optarg;
      break;
    case 'e':
      s_options.execute_script_path = optarg;
      break;
    case 'r':
      opts->document_root = optarg;
      printf("document_root=%s\r\n", opts->document_root);
      break;
    case 'p':
      s_options.mg_http_port = optarg;
      break;
    case 'q':
      s_options.api_handle_file_path = optarg;
      printf("api_handle_file_path=%s\r\n", s_options.api_handle_file_path);
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

  /* Open database */
  // if ((s_options.sqlite_handle = db_open(s_db_path)) == NULL) {
  //     fprintf(stderr, "Cannot open DB [%s]\n", s_db_path);
  //     exit(EXIT_FAILURE);
  // }

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
}

static void signal_handler(int sig_num) 
{
    signal(sig_num, signal_handler);
    s_options.signal = sig_num;
}

// #endregion options implement area
// ////////////////////////////////////////////////////////
// #region main function implement area

/**
 * 
 */
int main(int argc, char *argv[]) 
{
    // 解析命令行参数
    parse_options(argc,argv);

    // 如果是执行脚本, 执行并退出
    if (s_options.execute_script_path!=NULL)
      return js_execute_script(s_options.execute_script_path);

    // 初始化 mongoose web server
    mg_mgr_init(&s_options.mg_manager, NULL);
    s_options.mg_acceptor = mg_bind(&s_options.mg_manager, s_options.mg_http_port, event_handler);
    // {
      mg_set_protocol_http_websocket(s_options.mg_acceptor);
      mg_set_timer(s_options.mg_acceptor, mg_time() + SESSION_CHECK_INTERVAL);

      mg_register_http_endpoint(s_options.mg_acceptor, s_login_url, login_handler);
      mg_register_http_endpoint(s_options.mg_acceptor, s_logout_url, logout_handler);
    // }
    
    // 初始化libssh运行环境
    ssh_init();

    // 初始化quickjs运行环境
    qjs_runtime_init();
    
    printf("Starting server on port %s\r\n", s_options.mg_http_port);

    /* 进入主事件循环, Run event loop until signal is received */
    while (s_options.signal == 0) {
      // 因为mg的poll不包含ssh的socket, 所以就算ssh有数据, mg也是不知道
      // 只有当有输入或者超时事件到, 才会触发,, 所以需要设置得小一点, 同时ssh的接收缓冲区设置得大一点
      mg_mgr_poll(&s_options.mg_manager, 100);
    }

    printf("Exiting on signal %d\r\n", s_options.signal);

    /* Cleanup */
    mg_mgr_free(&s_options.mg_manager);

    // if(s_options.sqlite_handle) db_close(&s_options.sqlite_handle);

    // 释放quickjs运行环境资源
    qjs_runtime_free();

    // 释放libssh运行环境资源
    ssh_finalize();

    return 0;
}

// #endregion postgres declare area
// ////////////////////////////////////////////////////////
struct mg_str auth_mode_cookie = MG_MK_STR("cookie");
struct mg_str auth_mode_digest = MG_MK_STR("digest");
struct mg_str auth_mode_basic = MG_MK_STR("basic");
struct mg_str auth_mode_jwt = MG_MK_STR("jwt");

// 

char * clear_screen = "\x1B[2J";

static void event_handler(struct mg_connection *nc, int ev, void *ev_data) 
{
    switch (ev) {
    case MG_EV_HTTP_REQUEST:{ // 接收到http请求
        struct http_message *hm = (struct http_message *) ev_data;
        if (mg_str_has_prefix(&hm->uri, &api_prefix)) // 如果是API请求
        {
          if(check_authentication(nc, hm)==1){
            handle_api_request(nc, hm);
          }
        } else {
            mg_serve_http(nc, hm, s_options.mg_options); /* Serve static content */
        }
        break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:{
      struct http_message *hm = (struct http_message *) ev_data;
      // TODO 
      // if(check_authentication(nc, hm)==1){
        
      // }
      struct ws_ssh_context* context = new_ws_ssh_context(nc);
      nc->user_data = context;
      
      int len = 0;
      len = mg_get_http_var(&hm->query_string, "host", context->host, sizeof(context->host));
      if(len==0){
        mg_get_http_var(&hm->query_string, "h", context->host, sizeof(context->host));
      }
      len = mg_get_http_var(&hm->query_string, "port", context->port, sizeof(context->port));
      if(len==0){
        mg_get_http_var(&hm->query_string, "p", context->port, sizeof(context->port));
      }
      len = mg_get_http_var(&hm->query_string, "user", context->user, sizeof(context->user));
      if(len==0){
        mg_get_http_var(&hm->query_string, "u", context->user, sizeof(context->user));
      }
      len = mg_get_http_var(&hm->query_string, "password", context->password, sizeof(context->password));
      if(len==0){
        mg_get_http_var(&hm->query_string, "w", context->password, sizeof(context->password));
      }

      break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: { // WEBSOCKET握手完成, 建立连接成功
      struct ws_ssh_context* rs = nc->user_data;

      // 监视管道数据, 转发数据
      // please input connect target, format: [user[:password]@]hostname[:port]:
      // 支持的command:
      // help
      // list [pattern]
      // ssh [user[:password]@]hostname[:port]

      char * buf = "Connecting\x1B[1;3;31...\x1B[0m$\r\n";
      mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, buf, strlen(buf));
      
      // TODO 需要改成异步建立连接
      int rc = ws_ssh_connect(rs);
      if(rc!=0){
        buf = "Connect failed!\r\n";
        mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, buf, strlen(buf));
        mg_send_websocket_frame(nc, WEBSOCKET_OP_CLOSE, NULL, 0);
        free_ws_ssh_context(rs);
        nc->user_data = NULL;
      }else{
        // connected!!!
      }

      break;
    }
    case MG_EV_WEBSOCKET_FRAME: { // 接收到客户端发过来的消息, echo 回去
      struct ws_ssh_context* rs = nc->user_data;
      struct websocket_message *wm = (struct websocket_message *) ev_data;


      // 检查当前连接状态, 如果还没有建立后台ssh连接, 则解析命令
      // 因为是一个字母一个字母传过来的, 所以需要有缓冲区
      // 可以参考readline怎么写的
      // 直到遇到回车, 开始解析命令
      if(rs==NULL || rs->status!=1){
        
        printf("MG_EV_WEBSOCKET_FRAME>> %d\n", rs==NULL ? -1: rs->status);

        char * buf = "Error...Connection Closed! \r\n";
        mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, buf, strlen(buf));
        // mg_send_websocket_frame(nc, WEBSOCKET_OP_CLOSE, NULL,0);
      }else{
        ssh_channel_write(rs->channel, (void *) wm->data, wm->size);
      }

      // mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, (char *) wm->data, wm->size);
      // 转发数据到管道, libssh会从管道读取到数据
      // write(rs->ws_write_ssh_read[1], (char *) wm->data, wm->size);

      break;
    }
    case MG_EV_POLL: {
      if(is_websocket(nc)){
        struct ws_ssh_context* rs = nc->user_data;
        if(rs!=NULL && rs->status==1){
          if(ssh_channel_is_open(rs->channel) && ssh_channel_is_eof(rs->channel)==0){
            // 读取ssh的数据
            rs->ssh_read_buf_len = ssh_channel_read_nonblocking(rs->channel, rs->ssh_read_buf, rs->ssh_read_buf_size, true);
            if(rs->ssh_read_buf_len>0){
              mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, rs->ssh_read_buf, rs->ssh_read_buf_len);
            }else if(rs->ssh_read_buf_len == SSH_ERROR ){
              mg_send_websocket_frame(nc, WEBSOCKET_OP_CLOSE, NULL,0);
            }
            rs->ssh_read_buf_len = ssh_channel_read_nonblocking(rs->channel, rs->ssh_read_buf, rs->ssh_read_buf_size, false);
            if(rs->ssh_read_buf_len>0){
              mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, rs->ssh_read_buf, rs->ssh_read_buf_len);
            }else if(rs->ssh_read_buf_len == SSH_ERROR ){
              mg_send_websocket_frame(nc, WEBSOCKET_OP_CLOSE, NULL,0);
            }
          }
        }
      }
      break;
    }
    case MG_EV_CLOSE: {
      if(is_websocket(nc)){
        struct ws_ssh_context* rs = nc->user_data;
        free_ws_ssh_context(rs);
        rs = NULL;
        nc->user_data = NULL;
      }
    }
  }
}

// 1: 验证通过, 0: 不通过, -1: 不支持的认证方式
static int check_authentication(struct mg_connection *nc, struct http_message *hm){
  int rc = -1;

  struct mg_str* auth_mode = mg_get_http_header(hm, "Authentication-Mode");

  if(auth_mode==NULL || mg_str_is_equal(auth_mode, &auth_mode_cookie)){
    char ssid[32];
    int len = get_session_id(hm, ssid, sizeof(ssid));
    if(len){
        struct session *s = get_session(hm);
        if(s == NULL){
          send_cookie_auth_request(nc, "session expired.");
          rc = 0;
        }else{
          rc = 1;
        }
    }else{
      send_cookie_auth_request(nc, "please login.");
      rc = 0;
    }
  } else if(mg_str_is_equal(auth_mode, &auth_mode_digest)){
    if(mg_http_custom_is_authorized(
        hm, 
        s_options.mg_options.auth_domain, 
        s_options.mg_options.get_user_htpasswd_fn
    )){
      rc = 1;
    }else{
        mg_http_send_digest_auth_request(nc, s_options.mg_options.auth_domain);
        rc = 0;
    }
  }

  return rc;
}

static int ws_ssh_connect(struct ws_ssh_context *context)
{
    int auth = 0;
    char *banner;
    int state;

    ssh_session session = context->session;

    if (ssh_options_set(session, SSH_OPTIONS_USER, context->user) < 0) {
        fprintf(stderr, "Connection to [%s@%s:%s] failed: set user failed: %s\r\n", context->user, context->host, context->port, ssh_get_error(session));
        return -1;
    }
    if (ssh_options_set(session, SSH_OPTIONS_HOST, context->host) < 0) {
        fprintf(stderr, "Connection to [%s@%s:%s] failed : set host failed: %s\r\n", context->user, context->host, context->port, ssh_get_error(session));
        return -1;
    }
    if (strlen(context->port)>0 && ssh_options_set(session, SSH_OPTIONS_PORT_STR, context->port) < 0) {
        fprintf(stderr, "Connection to [%s@%s:%s] failed : set port failed: %s\r\n", context->user, context->host, context->port, ssh_get_error(session));
        return -1;
    }
    
    if (ssh_connect(session)) {
        fprintf(stderr, "Connection to [%s@%s:%s] failed : %s\r\n", context->user, context->host, context->port, ssh_get_error(session));
        return -1;
    }

    state = verify_knownhost(session);
    if (state != 0) {
        fprintf(stderr, "Connection to [%s@%s:%s] failed : verify_knownhost state=%d, error=%s\r\n", context->user, context->host, context->port, state, ssh_get_error(session));
        return -1;
    }
    
    if (ssh_authenticate(session, context->password) != SSH_AUTH_SUCCESS)
    {
      fprintf(stderr, "Authentication failed: %s\r\n", ssh_get_error(session));
      return -1;
    }

    ssh_channel channel = ssh_channel_new(session);
    if (channel == NULL) {
        printf("Error new channel : %s\r\n", ssh_get_error(session));
        return -2;
    }

    int rc = 0;
    rc = ssh_channel_open_session(channel);
    if(rc != SSH_OK){
        printf("Error opening channel : code=%d, error=%s\r\n", rc, ssh_get_error(session));
        ssh_channel_free(channel);
        return -2;
    }
    rc = ssh_channel_request_pty(channel);
    if(rc != SSH_OK){
        printf("Error request pty : code=%d, error=%s\r\n", rc, ssh_get_error(session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return -2;
    }
    rc = ssh_channel_request_shell(channel);
    if(rc != SSH_OK){
        printf("Requesting shell : code=%d, error=%s\r\n", rc, ssh_get_error(session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return -2;
    }

    context->channel = channel;
    context->status = 1;

    return 0;
}

static int ssh_authenticate(ssh_session session, char *password)
{
    int rc;
    int method;
    char *banner;

    // Try to authenticate
    rc = ssh_userauth_none(session, NULL);
    if (rc == SSH_AUTH_ERROR) {
        ssh_print_error(session);
        return rc;
    }

    method = ssh_userauth_list(session, NULL);
    ssh_print_supported_auth_methods(method);

    // Try to authenticate with password
    if (method & SSH_AUTH_METHOD_PASSWORD && password!=NULL) {
        rc = ssh_userauth_password(session, NULL, password);
        if (rc == SSH_AUTH_ERROR) {
            ssh_print_error(session);
            return rc;
        }
    }else if (method & SSH_AUTH_METHOD_GSSAPI_MIC){
        rc = ssh_userauth_gssapi(session);
        if(rc == SSH_AUTH_ERROR) {
            ssh_print_error(session);
            return rc;
        }
    } else if (method & SSH_AUTH_METHOD_PUBLICKEY) {
        rc = ssh_userauth_publickey_auto(session, NULL, NULL);
        if (rc == SSH_AUTH_ERROR) {
            ssh_print_error(session);
            return rc;
        }
    }

    if (rc != SSH_AUTH_SUCCESS) {
        return rc;
    }

    banner = ssh_get_issue_banner(session);
    if (banner) {
        printf("%s\r\n",banner);
        SSH_STRING_FREE_CHAR(banner);
    }

    return rc;
}

// 判断连接是否是websocket连接
static int is_websocket(const struct mg_connection *nc) {
  return nc->flags & MG_F_IS_WEBSOCKET;
}

// ////////////////////////////////////////////////////////
// #region API implement area

static void handle_api_request(struct mg_connection *nc, struct http_message *hm)
{

    // TODO: 开发阶段, 每次都重新加载脚本, 以便脚本修改后立马生效

    JSRuntime* runtime = JS_NewRuntime();
    if (!runtime) {
        fprintf(stderr, "qjs: cannot allocate JS runtime\n");
        exit(2);
    }
    JSContext* context = qjs_context_init(runtime);

    qjs_handle_api_request(context, nc, hm);

    qjs_context_free(context);
    js_std_free_handlers(runtime);
    JS_FreeRuntime(runtime);

    // // 需要改成用hashmap
    // char* path = (char*)(hm->uri.p);
    // path = path+api_prefix.len-1;
    // int length = hm->uri.len-api_prefix.len-4; // +1-strlen(".json")
    // if (strncmp(path, "/services/host", length)==0)
    // {
    //     // 从数据库读取节点的信息
    //     // 然后连接到host, 获取更进一步的信息
    // }
    

    // mg_serve_http(nc, hm, s_options.mg_options);
}

static void qjs_handle_api_request(JSContext* context, struct mg_connection *nc, struct http_message *hm)
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
        JSValue grobal = JS_GetGlobalObject(context);
        JSAtom fn_name = JS_NewAtom(context, s_options.api_handle_function);
        JSValue argv2[] = { request, response };

        JSValue rs = JS_Invoke(context, grobal, fn_name, 2, argv2);
        
        JS_FreeAtom(context, fn_name);
        JS_FreeValue(context, grobal);

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

        JS_FreeCString(context, status_text);
        if(!JS_IsNull(body_value)) JS_FreeCString(context, body);

        JS_FreeValue(context, status_value);
        JS_FreeValue(context, status_text_value);
        JS_FreeValue(context, headers_value);
        JS_FreeValue(context, body_value);
        
    }

    clean:
        JS_SetOpaque(request, NULL);
        JS_FreeValue(context, request);
        JS_FreeValue(context, response);
    
}

static int mg_str_has_prefix(const struct mg_str *uri, const struct mg_str *prefix) {
    return uri->len > prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static int mg_str_is_equal(const struct mg_str *s1, const struct mg_str *s2) {
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
    if(get_user_htpasswd(mg_mk_str(user), mg_mk_str(s_options.mg_options.auth_domain), ha1) && (strcmp(pass, ha1) == 0)){
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
    printf("Evicted %" INT64_X_FMT "/%s\r\n", oldest_s->id, oldest_s->user);
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
  context = qjs_context_init(runtime);

  return 0;
}

static JSContext* qjs_context_init(JSRuntime* runtime) 
{

  JSContext* context = JS_NewContext(runtime);
  if (!context) {
      fprintf(stderr, "qjs: cannot allocate JS context\n");
      exit(2);
  }

  /* loader for ES6 modules */
  JS_SetModuleLoaderFunc(runtime, NULL, js_module_loader, NULL);

  // build in functions and modules
  js_std_add_helpers(context, 0, NULL);
  js_init_module_std(context, "std");
  js_init_module_os(context, "os");

  /* make 'std' and 'os' visible to non module code */
  const char *str2 = "import %s from '%s';\n""globalThis.%s = %s;\n";
  char *str3 = (char *) malloc(strlen(str2)-8 + strlen(s_options.api_handle_file_path)+ (strlen(s_options.api_handle_function)*4 ));
  sprintf(str3, str2, s_options.api_handle_function, s_options.api_handle_file_path, s_options.api_handle_function, s_options.api_handle_function);
  eval_buf(context, str3, strlen(str3), "<input>", JS_EVAL_TYPE_MODULE);
  free(str3);

  JSValue grobal = JS_GetGlobalObject(context);
  JS_SetPropertyStr(context, grobal, "pg_connect_db", JS_NewCFunction(context, js_PQconnectdb, NULL, 0));
  JS_FreeValue(context, grobal);

  return context;
}

static int qjs_context_free(JSContext* context) 
{
  JS_FreeContext(context);
  return 0;
}

static int qjs_runtime_free() 
{
  // JS_FreeValue(context, grobal);
  JS_FreeContext(context);
  js_std_free_handlers(runtime);
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
  // printf("ERROR: %s\r\n", msg);
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
  printf("%s%s\r\n", prefix, msg);
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
      JS_SetPropertyStr(ctx, resultObj, "getColumnIndex", JS_NewCFunction(ctx, js_PQfnumber, NULL, 0));
      JS_SetPropertyStr(ctx, resultObj, "getColumnName", JS_NewCFunction(ctx, js_PQfname, NULL, 0));
      JS_SetPropertyStr(ctx, resultObj, "getColumnType", JS_NewCFunction(ctx, js_PQftype, NULL, 0));
      JS_SetPropertyStr(ctx, resultObj, "getColumnSize", JS_NewCFunction(ctx, js_PQfsize, NULL, 0));
       

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
static JSValue js_PQftype(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    PGresult* res = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    if(res==NULL)
      return JS_EXCEPTION;

    int col = JS_VALUE_GET_INT(argv[0]);
    Oid type = PQftype(res, col);
  return JS_NewInt32(ctx, type);
}
static JSValue js_PQfsize(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    PGresult* res = JS_GetOpaque(this_val, DEFAULT_JS_OBJECT_CLASS_ID);
    if(res==NULL)
      return JS_EXCEPTION;

    int col = JS_VALUE_GET_INT(argv[0]);
    int size = PQfsize(res, col);
  return JS_NewInt32(ctx, size);
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

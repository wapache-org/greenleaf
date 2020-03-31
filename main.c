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

#include <assert.h>

#include <sys/prctl.h>
#include <sys/time.h>

// header for postgresql
#include "libpq-fe.h"

// header for quickjs
#include "quickjs/quickjs-libc.h"

#include "libssh/callbacks.h"
#include "libssh/libssh.h"
#include "libssh/sftp.h"

// header for mongoose
#include "mongoose.h"
#include "ssh/ssh_common.h"

#include "crontab.h"
#include "common/logger.h"
#include "common/str_builder.h"
#include "common/array_list.h"
#include "common/hash_map.h"
#include "common/thread_pool.h"

#include "yaml.h"

#include "sqlite3.h"


// #endregion include area
// ////////////////////////////////////////////////////////////////////////////
// #region declare area

// ////////////////////////////////////////////////////////
// #region main declare area

// 短选项字符串, 一个字母表示一个短参数, 如果字母后带有冒号, 表示这个参数必须带有参数
// 建议按字母顺序编写
static char* short_opts = "c:d:e:f:hl:";
// 长选项字符串, 
// {长选项名字, 0:没有参数|1:有参数|2:参数可选, flags, 短选项名字}
// 建议按长选项字母顺序编写
static const struct option long_options[] = {
		{"crontab",1,NULL,'c'},
		{"database",1,NULL,'d'},
		{"execute",1,NULL,'e'},
		{"file"   ,1,NULL,'f'},
		{"help"   ,0,NULL,'h'},
		{"log-level"   ,1,NULL,'l'}
};
// 打印选项说明
static void usage(int argc, char* argv[])
{
  printf("Usages: \n");
  printf("    %s -c conf/crontab.json\n", argv[0]);
  printf("    %s -e qjs-modules/hello.js\n", argv[0]);
  printf("    %s -f conf/%s.yml\n", argv[0], argv[0]);
  printf("Options:\n");
  printf("    [-%s, --%s]     %s\n", "c", "crontab  ", "the contab json file, e.g. conf/crontab.json");
  printf("    [-%s, --%s]     %s\n", "d", "database ", "the embed database file, e.g. data/sqlite.db");
  printf("    [-%s, --%s]     %s\n", "e", "execute  ", "execute the script");
  printf("    [-%s, --%s]     %s\n", "f", "file     ", "the config file, default is conf/greenleaf.yml");
  printf("    [-%s, --%s]     %s\n", "h", "help     ", "print this message");
  printf("    [-%s, --%s]     %s\n", "l", "log-level", "set log level: trace,debug,info,warn,fatal,none");
}

// 解析选项
static void parse_cmd_options(int argc, char* argv[]);

// 
static void signal_handler(int sig_num);

// #endregion main declare area
// ////////////////////////////////////////////////////////
// #region mongoose declare area

static const struct mg_str s_get_method = MG_MK_STR("GET");
static const struct mg_str s_put_method = MG_MK_STR("PUT");
static const struct mg_str s_delele_method = MG_MK_STR("DELETE");

static void admin_event_handler(struct mg_connection *nc, int ev, void *ev_data);

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

// input username and auth_domain , output ha1
// return 1: found, 0 not found
typedef int (*mg_auth_get_user_htpasswd_fn)(struct mg_str username, struct mg_str auth_domain, char* out_ha1);

int mg_http_custom_is_authorized(struct http_message *hm, const char *domain, 
  mg_auth_get_user_htpasswd_fn get_user_htpasswd_fn);

int mg_http_custom_check_digest_auth(
  struct http_message *hm, 
  const char *auth_domain,
  mg_auth_get_user_htpasswd_fn mg_auth_get_user_htpasswd
);

int mg_custom_check_digest_auth(
    struct mg_str method, struct mg_str uri,
    struct mg_str username, struct mg_str cnonce,
    struct mg_str response, struct mg_str qop,
    struct mg_str nc, struct mg_str nonce,
    struct mg_str auth_domain, 
    mg_auth_get_user_htpasswd_fn mg_auth_get_user_htpasswd
);

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

// static JSValue js_sqlite_open(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
// static JSValue js_sqlite_close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_sqlite_exec(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
static JSValue js_sqlite_exec2(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

static JSValue js_get_physical_network_interface_io_speed(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

int callback_fn_get_physical_nic(any_t map_context, const char* key, any_t value);

int load_physical_network_interfaces();
int update_physical_network_interface_io_rate();
void* thread_fn_update_physical_network_interface_io_rate(void* ctx);


// 网卡列表 Map<网卡名称, struct nic_stat*>
map_t nic_map = NULL;

struct nic 
{
	struct timeval ts;
	long rx_bytes;
	long tx_bytes;
};

struct nic_stat
{
	char* name;
	long read; // B/s
	long write; // B/s

	struct nic* n0; //    指向 n1 或者 n2
	struct nic n1;
	struct nic n2;
};

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

// 心跳内容
static char* heartbeat_content = "heartbeat.";

// 事件处理器
static void multicast_event_handler(struct mg_connection *nc, int ev, void *p);

void * crontab_thread_proc(void *param);

// #endregion misc declare area
// ////////////////////////////////////////////////////////
// #endregion declare area
// ////////////////////////////////////////////////////////////////////////////
// #region implement area
// ////////////////////////////////////////////////////////
// #region options implement area

// 配置文件的配置项
struct conf_file_options
{
  map_t* config;
  struct 
  {
    char* path;
    char* sqlite;
    map_t* properties;
  } storage;

  struct 
  {
    int enabled;
    char* level;
    map_t* loggers;
  } logger;

  struct 
  {
    int enabled;
    char* path;
  } crontab;

  struct 
  {
    int enabled;
    arraylist* ducument_roots;
    char*      bind_address;
    arraylist* node_addresses;
    char* api_prefix;
    char* api_router;
    char* api_handler;
    char* auth_domain;
    // {"yes", "no"}
    char* enable_directory_listing;
  } admin;

  int group_count;
  struct group_conf {
    int enabled;
    char* name; 

    char* local; // 格式: IP, 空字符串表示使用 INADDR_ANY

    char* group; // multicast group 地址, 格式: IP
    int port;  // multicast group 端口 

    int heartbeat; // 心跳周期, 单位秒, 取值范围:[1,60]
  } groups[8]; // struct multicast_group

  arraylist * services;

};

// 对应greenpleaf配置文件中的groups配置项
struct multicast_context
{
    int enabled;
    char* name; 
    int heartbeat;

  // 组播"连接"
  struct mg_connection *sender;
  struct mg_connection *receiver;

  char* status;
};

// 管理服务
struct admin_context
{

  struct mg_connection * mg_acceptor;
  struct mg_serve_http_opts mg_options;

};

// 命令行上下文
struct cmd_context {

  char * log_level;

  char * sqlite_path;

  char * execute_script_path;

  char * crontab_file_path;
  crontab* crontab;

  char * conf_file_path;
  struct conf_file_options conf_file_options;

  int signal;

  struct mg_mgr event_loop_manager;

  struct admin_context admin_context;

  int group_count;
  struct multicast_context* multicast_contexts[32];

  sqlite3 * sqlite;

};

// 命令行上下文全局变量
static struct cmd_context s_context;

static void signal_handler(int sig_num) 
{
//     printf("\n Cannot be terminated using Ctrl+C \n"); 
//     fflush(stdout); 
    signal(sig_num, signal_handler);
    // signal(sig_num, SIG_IGN);
    s_context.signal = sig_num;
}

static void parse_cmd_options(int argc, char* argv[])
{
  // 先设置默认值
  s_context.sqlite_path = NULL;
  s_context.execute_script_path = NULL;
  s_context.sqlite = NULL;
  s_context.log_level = "none";

  s_context.conf_file_path = "conf/greenleaf.yml";
  struct conf_file_options* conf = &s_context.conf_file_options;

  struct mg_serve_http_opts* opts = &s_context.admin_context.mg_options;
  opts->document_root = "static";
  opts->enable_directory_listing = "no";
  opts->auth_domain = "localhost";

  // 如果只有短选项, 用getopt就够了
  // int code = getopt(argc, argv, short_opts);

  // 解析长选项和短选项
  int opt = 0;
  while((opt=getopt_long(argc,argv,short_opts,long_options,NULL))!=-1){
    switch (opt)
    {
    case 'c':
      s_context.crontab_file_path = optarg;
      break;
    case 'd':
      s_context.sqlite_path = optarg;
      break;
    case 'e':
      s_context.execute_script_path = optarg;
      break;
    case 'f':
      s_context.conf_file_path = optarg;
      break;
    case 'h':
      usage(argc, argv);
      exit(EXIT_SUCCESS);
      break;
    case 'l':
      s_context.log_level = optarg;
      break;
    default:
      usage(argc, argv);
      exit(EXIT_FAILURE);
      break;
    }
  }
}

// TODO : yaml的api还不熟悉, 这个函数有内存泄漏, 逻辑也很混乱, 需要重写.
// 考虑先转成json, 然后再转成结构体
int parse_conf_file()
{
  int rc = 0;

  struct conf_file_options* conf = &s_context.conf_file_options;

  FILE *file;
  yaml_parser_t parser;
  yaml_char_t* value = NULL;
  char* level_1_key = NULL;
  char* level_2_key = NULL;
  char* level_3_key = NULL;

  arraylist* list = NULL;
  map_t* map = NULL;
  char* map_key = NULL;

  int level = 0;

  file = fopen(s_context.conf_file_path, "rb");
  assert(file);

  assert(yaml_parser_initialize(&parser));
  yaml_parser_set_input_file(&parser, file);

  yaml_event_t  event;   /* New variable */
  /* START new code */
  do {
    if (!yaml_parser_parse(&parser, &event)) {
       logger_fatal("load config file failed: Parser error %d", parser.error);
       return rc;
    }

    switch(event.type)
    { 
    case YAML_NO_EVENT: 
      logger_trace("No event!"); 
    break;
    /* Stream start/end */
    case YAML_STREAM_START_EVENT: logger_trace("STREAM START"); break;
    case YAML_STREAM_END_EVENT:   logger_trace("STREAM END");   break;
    /* Block delimeters */
    case YAML_DOCUMENT_START_EVENT: 
      logger_trace("<b>Start Document</b>"); 
    break;
    case YAML_DOCUMENT_END_EVENT:   logger_trace("<b>End Document</b>");   break;
    case YAML_SEQUENCE_START_EVENT: 
      level++;
      logger_trace("Start Sequence, level=%d", level); 

      list = NULL;
      map = NULL;
      if (level==3 && STR_IS_EQUALS(level_2_key,"groups"))
      {
        // list = arraylist_new(NULL);
        // logger_debug("map %s put %s list", level_1_key, level_2_key);
      }else if (level==4 && STR_IS_EQUALS(level_2_key,"admin") && map_key!=NULL){
        if(STR_IS_EQUALS(map_key,"ducument_roots")){
          list = conf->admin.ducument_roots = arraylist_new(NULL);
          logger_debug("map %s put %s list", level_2_key, map_key);
        }else if(STR_IS_EQUALS(map_key,"node_addresses")){
          list = conf->admin.node_addresses = arraylist_new(NULL);
          logger_debug("map %s put %s list", level_2_key, map_key);
        }
      }
    break;
    case YAML_SEQUENCE_END_EVENT:   
      logger_trace("Start Sequence, level=%d", level); 
      level--;
      list = NULL;
      map = NULL;
      map_key=NULL;

    break;
    case YAML_MAPPING_START_EVENT:  
      level++;
      logger_trace("Start Mapping, level=%d", level); 

      map = NULL;
      map_key=NULL;
      if (level==3){
        if( STR_IS_EQUALS(level_2_key,"config") ) {
          map = conf->config = hashmap_new();
        } else
        if( STR_IS_EQUALS(level_2_key,"storage") ) {
          map = conf->storage.properties = hashmap_new();
        } else
        if( STR_IS_EQUALS(level_2_key,"logger") ) {
          map = conf->logger.loggers = hashmap_new();
        }
      }else if(level==4){
      }

    break;
    case YAML_MAPPING_END_EVENT:    
      logger_trace("End Mapping, level=%d", level); 
      if(level==4){
        if( STR_IS_EQUALS(level_2_key,"groups") ){
          conf->group_count++;
        }
      }

      level--;
      list = NULL;
      map = NULL;
      map_key=NULL;

    break;
    /* Data */
    case YAML_ALIAS_EVENT:  
      logger_trace("Got alias (anchor %s)", event.data.alias.anchor); 
    break;
    case YAML_SCALAR_EVENT: 
      value = event.data.scalar.value;
      logger_trace("Got scalar (value %s)", value); 
      switch(level){
      case 1:{
        level_1_key = new_string(value);
        logger_debug("level 1 %s", level_1_key);
        // if(*opt==NULL && memcmp(level_1_key,"greenleaf", strlen("greenleaf"))==0){
        //   *opt = calloc(1, sizeof(struct greenpleaf_options));
        //   puts("====================>>>>>>>>>>>>Start greenleaf");  
        // }
      }
      break;
      case 2:{
        level_2_key = new_string(value);
        logger_debug("level 2 %s", level_2_key);
      }
      break;
      case 3:{
        level_3_key = new_string(value);
        if(list!=NULL){
          logger_debug("list %s add %s", level_2_key, value);
          arraylist_add(list, new_string(value));
        } else if(map!=NULL){
          if(map_key==NULL){
            map_key = new_string(value);
          }else{
            logger_debug("map %s put %s, %s", level_2_key, map_key, value);
            
            if( STR_IS_EQUALS(level_2_key,"logger") ) {
              if( STR_IS_EQUALS(map_key,"enabled") ) {
                conf->logger.enabled = STR_IS_EQUALS(value,"true") ? 1 : 0;
              }else
              if( STR_IS_EQUALS(map_key,"level") ) {
                conf->logger.level = new_string(value);
              }
            }

            if( STR_IS_EQUALS(level_2_key,"storage") ) {
              if( STR_IS_EQUALS(map_key,"path") ) {
                conf->storage.path = new_string(value);
              }else
              if( STR_IS_EQUALS(map_key,"sqlite") ) {
                conf->storage.sqlite = new_string(value);
              }
            }
            
            hashmap_put(map, map_key, new_string(value));

            map_key = NULL;
          }
        }else{
          // logger_debug("level_2_key %s \n", level_2_key);
          if( STR_IS_EQUALS(level_2_key,"admin") ) {
            if(map_key==NULL){
              map_key = new_string(value);
            }else{
              if( STR_IS_EQUALS(map_key,"enabled") ) {
                logger_debug("map %s put %s, %s", level_2_key, map_key, value);
                conf->admin.enabled = STR_IS_EQUALS(value,"true") ? 1 : 0;
              }else if( STR_IS_EQUALS(map_key,"bind_address") ) {
                logger_debug("map %s put %s, %s", level_2_key, map_key, value);
                conf->admin.bind_address = new_string(value);
              }else if( STR_IS_EQUALS(map_key,"api_prefix") ) {
                logger_debug("map %s put %s, %s", level_2_key, map_key, value);
                conf->admin.api_prefix = new_string(value);
              }else if( STR_IS_EQUALS(map_key,"api_router") ) {
                logger_debug("map %s put %s, %s", level_2_key, map_key, value);
                conf->admin.api_router = new_string(value);
              }else if( STR_IS_EQUALS(map_key,"api_handler") ) {
                logger_debug("map %s put %s, %s", level_2_key, map_key, value);
                conf->admin.api_handler = new_string(value);
              }else if( STR_IS_EQUALS(map_key,"auth_domain") ) {
                logger_debug("map %s put %s, %s", level_2_key, map_key, value);
                conf->admin.auth_domain = new_string(value);
              }else if( STR_IS_EQUALS(map_key,"enable_directory_listing") ) {
                logger_debug("map %s put %s, %s", level_2_key, map_key, value);
                conf->admin.enable_directory_listing = STR_IS_EQUALS(value,"yes") ? "yes" : "no";
              }
              map_key = NULL;
            }
          }else 
          if( STR_IS_EQUALS(level_2_key,"crontab") ) {
            if(map_key==NULL){
              map_key = new_string(value);
            }else{
              if( STR_IS_EQUALS(map_key,"enabled")) {
                logger_debug("map %s put %s, %s", level_2_key, map_key, value);
                conf->crontab.enabled = STR_IS_EQUALS(value,"true") ? 1 : 0;
              } else if( STR_IS_EQUALS(map_key,"path") ) {
                logger_debug("map %s put %s, %s", level_2_key, map_key, value);
                conf->crontab.path = new_string(value);
              }
              map_key = NULL;
            }
          }
        }
      }
      break;
      case 4:{
        if(list!=NULL){
          logger_debug("list %s add %s", level_3_key, value);
          arraylist_add(list, new_string(value));
        }

        if( STR_IS_EQUALS(level_2_key,"groups") ){
          struct group_conf* g = &conf->groups[conf->group_count];

          if(map_key==NULL){
            map_key = new_string(value);
          }else{
            logger_debug("group %s[%d] put %s, %s", level_2_key, conf->group_count, map_key, value);
            
            if( STR_IS_EQUALS(map_key,"enabled") ) {
              g->enabled = STR_IS_EQUALS(value,"true");
            }else if( STR_IS_EQUALS(map_key,"name") ) {
              g->name = new_string(value);
            }else if( STR_IS_EQUALS(map_key,"local") ) {
              g->local = new_string(value);
            }else if( STR_IS_EQUALS(map_key,"group") ) {
              g->group = new_string(value);
            }else if( STR_IS_EQUALS(map_key,"port") ) {
              g->port = atoi(new_string(value));
            }else if( STR_IS_EQUALS(map_key,"heartbeat") ) {
              g->heartbeat = atoi(new_string(value));
            }
            map_key = NULL;
          }
        }

      }break;
      default:{
      }
      }
    }
    if(event.type != YAML_STREAM_END_EVENT)
      yaml_event_delete(&event);
  } while(event.type != YAML_STREAM_END_EVENT);
  yaml_event_delete(&event);
  /* END new code */

  // yaml_document_delete(document);

  yaml_parser_delete(&parser);

  assert(!fclose(file));

  return rc;
}

// #endregion options implement area
// ////////////////////////////////////////////////////////
// #region main function implement area

void start_admin_process();
int start_main_loop();
int execute_script();
int execute_crontab();

/**
 * 
 */
int main(int argc, char *argv[]) 
{
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  // 解析命令行参数
  parse_cmd_options(argc,argv);

  logger_set_level_by_name(s_context.log_level);
  cs_log_set_level(LL_INFO); // _LL_MAX

  // 如果只是执行脚本, 执行并退出
  if (s_context.execute_script_path!=NULL) {
    return execute_script();
  }

  // 如果只是执行定时任务, 执行并退出
  if (s_context.crontab_file_path!=NULL) {
    return execute_crontab();
  }

  // 如果是读取配置文件, 解析并执行
  if(parse_conf_file()){
    exit(EXIT_FAILURE);
  }

  struct conf_file_options* conf = &s_context.conf_file_options;
  // 设置日志
  if(conf->logger.enabled){
    logger_set_level_by_name(conf->logger.level);
  }else{
    logger_set_level(LOG_NONE);
  }

  if(conf->admin.enabled) {
    pid_t child_pid = fork(); // fork进程, 启动管理服务
    if (child_pid == 0){
			logger_info("This is child process, parent process id is: %d, child process id is: %d", getppid(), getpid());
      // child process
      start_admin_process();
      exit(EXIT_SUCCESS);
    }else{
			logger_info("This is parent process, parent process id is: %d, child process id is: %d", getpid(), child_pid);
      // parent process
      prctl(PR_SET_PDEATHSIG,SIGHUP); // 防止父进程意外退出, 子进程成了野生进程
    }
  }

  // 进入主事件循环
  start_main_loop();

  return 0;
}


int execute_crontab()
{
  int rc = 0;

  if(s_context.sqlite_path!=NULL){
    if ((rc=sqlite3_open(s_context.sqlite_path, &s_context.sqlite)) ) {
        logger_fatal("Cannot open DB [%s]", s_context.sqlite_path);
        return rc;
    }else{
      logger_info("Open DB [%s] success.", s_context.sqlite_path);
    }
  }

  crontab_thread_proc(NULL);

  if(s_context.sqlite!=NULL){
    if ((rc=sqlite3_close(s_context.sqlite)) ) {
      logger_fatal("Cannot close DB [%s]", s_context.sqlite_path);
      return rc;
    }else{
      logger_info("Close DB [%s] success.", s_context.sqlite_path);
    }
  }

  return rc;
}

int execute_script()
{
  int rc = 0;

  if(s_context.sqlite_path!=NULL){
    if ((rc=sqlite3_open(s_context.sqlite_path, &s_context.sqlite)) ) {
        logger_fatal("Cannot open DB [%s]", s_context.sqlite_path);
        return rc;
    }else{
      logger_info("Open DB [%s] success.", s_context.sqlite_path);
    }
  }

  rc = js_execute_script(s_context.execute_script_path);

  if(s_context.sqlite!=NULL){
    if ((rc=sqlite3_close(s_context.sqlite)) ) {
      logger_fatal("Cannot close DB [%s]", s_context.sqlite_path);
      return rc;
    }else{
      logger_info("Close DB [%s] success.", s_context.sqlite_path);
    }
  }

  return rc;
}

void start_admin_process(){

  struct mg_mgr* mgr = &s_context.event_loop_manager;
  struct admin_context* ctx = &s_context.admin_context;

  struct mg_serve_http_opts* mg_opts = &ctx->mg_options;
  struct conf_file_options* gl_opts = &s_context.conf_file_options;

    // 初始化 mongoose web server
  mg_mgr_init(mgr, NULL);
  struct mg_connection * nc = mg_bind(mgr, gl_opts->admin.bind_address, admin_event_handler);
  // {
    if (nc == NULL) {
      logger_fatal("admin proces, binding failed: %s", gl_opts->admin.bind_address);
      exit(EXIT_FAILURE);
    }
    mg_set_protocol_http_websocket(nc);
    mg_set_timer(nc, mg_time() + SESSION_CHECK_INTERVAL);
    // TODO: when auth method support cookie
    mg_register_http_endpoint(nc, s_login_url, login_handler);
    mg_register_http_endpoint(nc, s_logout_url, logout_handler);
  // }
  ctx->mg_acceptor = nc;

  mg_opts->enable_directory_listing = gl_opts->admin.enable_directory_listing;
  if( ! arraylist_is_empty(gl_opts->admin.ducument_roots)){
    mg_opts->document_root = arraylist_get_idx(gl_opts->admin.ducument_roots, 0);
    arraylist_del_idx(gl_opts->admin.ducument_roots,0,1);
    mg_opts->document_roots = (const char**)arraylist_toarray(gl_opts->admin.ducument_roots);
  }
  mg_opts->auth_domain = gl_opts->admin.auth_domain;

  if(gl_opts->storage.sqlite!=NULL && strlen(gl_opts->storage.sqlite)>0 ){

    char path[256]={0};
    snprintf(path, sizeof(path), "%s/%s",
      gl_opts->storage.path,
      gl_opts->storage.sqlite
    );
    
    if (sqlite3_open(path, &s_context.sqlite) ) {
      fprintf(stderr, "Cannot open DB [%s]\n", path);
      exit(EXIT_FAILURE);
    }

  }

  // 初始化libssh运行环境
  ssh_init();

  // 初始化quickjs运行环境
  qjs_runtime_init();

  logger_info("Starting admin server on %s\r\n", gl_opts->admin.bind_address);

  /* 进入主事件循环, Run event loop until signal is received */
  while (s_context.signal == 0) {
    // 因为mg的poll不包含ssh的socket, 所以就算ssh有数据, mg也是不知道
    // 只有当有输入或者超时事件到, 才会触发,, 所以需要设置得小一点, 同时ssh的接收缓冲区设置得大一点
    mg_mgr_poll(mgr, 100);
  }

  logger_info("Exiting on signal %d\r\n", s_context.signal);

  /* Cleanup */
  mg_mgr_free(mgr);

  // 释放quickjs运行环境资源
  qjs_runtime_free();

  // 释放libssh运行环境资源
  ssh_finalize();

  if (s_context.sqlite) {
    sqlite3_close(s_context.sqlite);
  }

}

int start_main_loop() 
{

  struct mg_mgr* mgr = &s_context.event_loop_manager;
  
  struct conf_file_options* gl_opts = &s_context.conf_file_options;

  // 启动定时任务, 如果不运行管理服务, 则在主线程运行, 否则新开一个线程执行
  if(gl_opts->crontab.enabled){
    mg_start_thread(crontab_thread_proc, NULL);
  }

  mg_mgr_init(mgr, NULL);

  // join multicast groups and send heartbeat.
  // arraylist* groups = gl_opts->groups;
  // int group_count = arraylist_length(groups);
  for (size_t i = 0; i < gl_opts->group_count; i++)
  {
    struct group_conf* conf = &gl_opts->groups[i];

    struct multicast_context* ctx = calloc(1,sizeof(struct multicast_context));
    ctx->enabled = conf->enabled;
    ctx->name = conf->name;
    ctx->heartbeat = conf->heartbeat;
    
    s_context.multicast_contexts[i] = ctx;

    if(!ctx->enabled) {
      logger_debug("multicast group %s is disabled, skip it.", conf->name);
      continue;
    } else {
      logger_debug("multicast group %s: joining...", conf->name);
    }

    // receive 
    char listen[256];
    snprintf(listen, sizeof(listen), "udp://%d", conf->port);
    ctx->receiver = mg_bind(mgr, listen, multicast_event_handler);
    if (ctx->receiver == NULL) {
      //perror("cannot bind\n");
      logger_error("multicast group %s: cannot bind to %s", conf->name, listen);
      continue;
    }
    ctx->receiver->user_data = ctx;

    struct ip_mreq req;
    req.imr_multiaddr.s_addr = inet_addr(conf->group);
    req.imr_interface.s_addr = conf->local==NULL || strlen(conf->local)==0 || STR_IS_EQUALS(conf->local, "INADDR_ANY") 
      ? htonl(INADDR_ANY) : inet_addr(conf->local);
    if (setsockopt(ctx->receiver->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &req, sizeof(req)) < 0) {
      //perror("Adding multicast group error");
      logger_error("multicast group %s: add membership failed , multiaddr=%s, interface=%s", conf->name, conf->group, conf->local);
      continue;
    }
    logger_debug("multicast group %s: receiver added membership on %s:%d", conf->name, conf->group, conf->port);

    // send
    char group_address[256];
    snprintf(group_address, sizeof(group_address), "udp://%s:%d", conf->group, conf->port);
    ctx->sender = mg_connect(mgr, group_address, multicast_event_handler);
    if (ctx->sender == NULL) {
      // perror("cannot connect multicast address\n");
      logger_error("multicast group %s: sender cannot connect multicast address = %s", conf->name, group_address);
      continue;
    }
    ctx->sender->user_data = ctx;
    logger_debug("multicast group %s: sender connect on %s", conf->name, group_address);

    mg_set_timer(ctx->sender, mg_time() + conf->heartbeat);

    logger_info("multicast group %s: joined on %s", conf->name, group_address);
  }

  mg_start_thread(thread_fn_update_physical_network_interface_io_rate, NULL);

  logger_info("Start main event loop");
  while (s_context.signal == 0) {
    mg_mgr_poll(mgr, 1000);
  }

  mg_mgr_free(mgr);

  logger_info("Exiting main event loop on signal %d", s_context.signal);

  return 0;
}

void* thread_fn_update_physical_network_interface_io_rate(void* ctx)
{
  logger_debug("Start update_physical_network_interface thread");
  load_physical_network_interfaces();
  while (s_context.signal == 0) {
    update_physical_network_interface_io_rate();
    sleep(1);
  }
  logger_debug("Stop update_physical_network_interface thread");
  return NULL;
}

static void multicast_event_handler(struct mg_connection *nc, int ev, void *p) {
  (void) p;
  switch (ev) {
    case MG_EV_TIMER: {// 定时发送心跳
      // if(group_sender == nc) {
        struct multicast_context* g = nc->user_data;
        mg_send(nc, heartbeat_content, strlen(heartbeat_content));
        mg_set_timer(nc, mg_time() + g->heartbeat);
    }  // }
    break;
    case MG_EV_SEND: { // 心跳发送成功
      // if(group_sender == nc) {
        const char *peer = inet_ntoa(nc->sa.sin.sin_addr);
        uint16_t port = ntohs(nc->sa.sin.sin_port);
        printf("%f Heartbeat Sended to %s:%d\n", mg_time(), peer, port);
      // }
    }
    break;
    case MG_EV_RECV:{ // 接收心跳成功
      const char *peer = inet_ntoa(nc->sa.sin.sin_addr);
      uint16_t port = ntohs(nc->sa.sin.sin_port);
      struct mbuf *io = &nc->recv_mbuf;
      printf("%f Received (%zu bytes): '%.*s' from %s:%d\n", mg_time(), 
        io->len, (int) io->len, io->buf, 
        peer, port
      );
      mbuf_remove(io, io->len);
      nc->flags |= MG_F_SEND_AND_CLOSE; // udp没有连接, 接收到就是处理完, 就需要关闭了
    }
    break;
    case MG_EV_POLL: {
      // if(group_receiver != nc && group_sender != nc){
      //   const char *peer = inet_ntoa(nc->sa.sin.sin_addr);
      //   printf("poll: nc->sa: %s %d\n", peer, ntohs(nc->sa.sin.sin_port));
      // }
    }
    break;
    default:
      break;
  }
}

// #endregion postgres declare area
// ////////////////////////////////////////////////////////
struct mg_str auth_mode_cookie = MG_MK_STR("cookie");
struct mg_str auth_mode_digest = MG_MK_STR("digest");
struct mg_str auth_mode_basic = MG_MK_STR("basic");
struct mg_str auth_mode_jwt = MG_MK_STR("jwt");

// 

char * clear_screen = "\x1B[2J";

static void admin_event_handler(struct mg_connection *nc, int ev, void *ev_data) 
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
            mg_serve_http(nc, hm, s_context.admin_context.mg_options); /* Serve static content */
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
        s_context.admin_context.mg_options.auth_domain, 
        get_user_htpasswd
    )){
      rc = 1;
    }else{
        mg_http_send_digest_auth_request(nc, s_context.admin_context.mg_options.auth_domain);
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
    

    // mg_serve_http(nc, hm, s_context.admin_context.mg_options);
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
        JSAtom fn_name = JS_NewAtom(context, s_context.conf_file_options.admin.api_handler);
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


int mg_http_custom_check_digest_auth(struct http_message *hm, const char *auth_domain,
    mg_auth_get_user_htpasswd_fn mg_auth_get_user_htpasswd
) {
  int ret = 0;
  struct mg_str *hdr;
  char username_buf[50], cnonce_buf[64], response_buf[40], uri_buf[200],
      qop_buf[20], nc_buf[20], nonce_buf[16];

  char *username = username_buf, *cnonce = cnonce_buf, *response = response_buf,
       *uri = uri_buf, *qop = qop_buf, *nc = nc_buf, *nonce = nonce_buf;

  /* Parse "Authorization:" header, fail fast on parse error */
  if (hm == NULL || mg_auth_get_user_htpasswd == NULL ||
      (hdr = mg_get_http_header(hm, "Authorization")) == NULL ||
      mg_http_parse_header2(hdr, "username", &username, sizeof(username_buf)) ==
          0 ||
      mg_http_parse_header2(hdr, "cnonce", &cnonce, sizeof(cnonce_buf)) == 0 ||
      mg_http_parse_header2(hdr, "response", &response, sizeof(response_buf)) ==
          0 ||
      mg_http_parse_header2(hdr, "uri", &uri, sizeof(uri_buf)) == 0 ||
      mg_http_parse_header2(hdr, "qop", &qop, sizeof(qop_buf)) == 0 ||
      mg_http_parse_header2(hdr, "nc", &nc, sizeof(nc_buf)) == 0 ||
      mg_http_parse_header2(hdr, "nonce", &nonce, sizeof(nonce_buf)) == 0 ||
      mg_check_nonce(nonce) == 0) {
    ret = 0;
    goto clean;
  }

  /* NOTE(lsm): due to a bug in MSIE, we do not compare URIs */

  ret = mg_custom_check_digest_auth(
      hm->method,
      mg_mk_str_n(
          hm->uri.p,
          hm->uri.len + (hm->query_string.len ? hm->query_string.len + 1 : 0)),
      mg_mk_str(username), mg_mk_str(cnonce), mg_mk_str(response),
      mg_mk_str(qop), mg_mk_str(nc), mg_mk_str(nonce), mg_mk_str(auth_domain),
      mg_auth_get_user_htpasswd);

clean:
  if (username != username_buf) free(username);
  if (cnonce != cnonce_buf) free(cnonce);
  if (response != response_buf) free(response);
  if (uri != uri_buf) free(uri);
  if (qop != qop_buf) free(qop);
  if (nc != nc_buf) free(nc);
  if (nonce != nonce_buf) free(nonce);

  return ret;
}

int mg_custom_check_digest_auth(
    struct mg_str method, struct mg_str uri,
    struct mg_str username, struct mg_str cnonce,
    struct mg_str response, struct mg_str qop,
    struct mg_str nc, struct mg_str nonce,
    struct mg_str auth_domain, 
    mg_auth_get_user_htpasswd_fn mg_auth_get_user_htpasswd
) {
  char f_ha1[128];
  char exp_resp[33];

  if (mg_auth_get_user_htpasswd(username, auth_domain, f_ha1)) {
      /* Username and domain matched, check the password */
      mg_mkmd5resp(method.p, method.len, uri.p, uri.len, f_ha1, strlen(f_ha1),
                   nonce.p, nonce.len, nc.p, nc.len, cnonce.p, cnonce.len,
                   qop.p, qop.len, exp_resp);
      logger_debug("%.*s %s %.*s %s", (int) username.len, username.p,
                     auth_domain, (int) response.len, response.p, exp_resp);
      return mg_ncasecmp(response.p, exp_resp, strlen(exp_resp)) == 0;
  }

  /* None of the entries matched - return failure */
  return 0;
}
int mg_http_custom_is_authorized(
  struct http_message *hm, const char *domain, 
  mg_auth_get_user_htpasswd_fn get_user_htpasswd_fn
){
  return mg_http_custom_check_digest_auth(hm, domain, get_user_htpasswd_fn);
}



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
    if(get_user_htpasswd(mg_mk_str(user), mg_mk_str(s_context.admin_context.mg_options.auth_domain), ha1) && (strcmp(pass, ha1) == 0)){
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
  const char* fn = s_context.conf_file_options.admin.api_handler;
  const char* path = s_context.conf_file_options.admin.api_router;
  const char *str2 = "import %s from '%s';\n""globalThis.%s = %s;\n";
  char *str3 = (char *) malloc(strlen(str2)-8 + strlen(path)+ (strlen(fn)*4 ));
  sprintf(str3, str2, fn, path, fn, fn);
  eval_buf(context, str3, strlen(str3), "<input>", JS_EVAL_TYPE_MODULE);
  free(str3);

  JSValue grobal = JS_GetGlobalObject(context);
  JS_SetPropertyStr(context, grobal, "pg_connect_db", JS_NewCFunction(context, js_PQconnectdb, NULL, 0));
  JS_SetPropertyStr(context, grobal, "sqlite_exec", JS_NewCFunction(context, js_sqlite_exec, NULL, 0));
  JS_SetPropertyStr(context, grobal, "sqlite_exec2", JS_NewCFunction(context, js_sqlite_exec2, NULL, 0));
  JS_SetPropertyStr(context, grobal, "sys_get_pni_speed", JS_NewCFunction(context, js_get_physical_network_interface_io_speed, NULL, 0));
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
    // 是不是可以直接: JS_NewCFunction(ctx, js_PQconnectdb, "pg_connect_db", 0) ???
    JS_SetPropertyStr(ctx, g, "pg_connect_db", JS_NewCFunction(ctx, js_PQconnectdb, NULL, 0));
    JS_SetPropertyStr(ctx, g, "sqlite_exec", JS_NewCFunction(ctx, js_sqlite_exec, NULL, 0));
    JS_SetPropertyStr(ctx, g, "sqlite_exec2", JS_NewCFunction(ctx, js_sqlite_exec2, NULL, 0));
    JS_SetPropertyStr(ctx, g, "sys_get_pni_speed", JS_NewCFunction(ctx, js_get_physical_network_interface_io_speed, NULL, 0));
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
// #region crontab implement area

static threadpool crontab_thread_pool = NULL;
int crontab_job_trigger_callback( crontab_job* job, void *user_data);
void * crontab_thread_proc(void *param) {
  // struct mg_mgr *mgr = (struct mg_mgr *) param;
  
  logger_info("the crontab service started.");

  const char* err;
  crontab* crontab = NULL;

  if(crontab_new(&crontab)){
      goto free;
  }
  
  if(crontab_load(crontab)){
      goto free;
  }

  // crontab_job* job = NULL;

  // if(crontab_new_job(&job)){
  //     goto free;
  // }

  // job->name = new_string("job1");
  // job->action = new_string("pg_vacuum");
  // job->payload = new_string("sys_user,sys_role");
  // job->cron_expr = cronexpr_parse("0/3 * * * * *", &err);

  // if(crontab_add_job(crontab, job)){
  //     goto free;
  // }

  load_physical_network_interfaces();

  int thread_count = crontab_get_job_count(crontab);
  crontab_thread_pool = thpool_init("crontab", thread_count*2);
  while (s_context.signal == 0) {
    sleep(1);
    update_physical_network_interface_io_rate();
    crontab_iterate(crontab, crontab_job_trigger_callback, NULL);
  }
  if(crontab_thread_pool!=NULL){
    thpool_destroy(crontab_thread_pool);
  }

free:
    crontab_free(crontab);

  logger_info("the crontab service stoped.");
  fflush(stdout);
  return NULL;
}

void execute_script_in_thread(void* script_file)
{
  js_execute_script((const char*)script_file);
}

// 自动调用fork,调用/bin/sh -c 执行cmd命令, 直到命令执行完后才返回
int execute_cmd_by_system(const char *cmd)
{
  int rc = system(cmd);
  return rc;
}

// 自动调用fork,调用/bin/sh -c 执行cmd命令并且建立管道, 可以读取命令执行的输出
void execute_cmd_by_popen(const char *cmd, struct str_builder *sb)
{
  char buf_ps[1024];
  FILE *ptr;
  if((ptr=popen(cmd, "r"))!=NULL)   
  {
      while(fgets(buf_ps, 1024, ptr)!=NULL)   
      {
        str_builder_add_str(sb, buf_ps, 0);  
      }
      pclose(ptr);   
      ptr = NULL;   
  }   
  else  
  {   
      printf("popen `%s` error\n", cmd);   
  }
}

// 如果要重定向输入和输出, 重定向1,2文件描述符
void execute_cmd_by_execve(const char *path, char *const argv[], char *const envp[])   
{
  if(fork()==0)
  {
    int rc = execve(path,argv,envp);
    exit(rc);
  }
}

#define CRON_INVALID_INSTANT -1
int crontab_job_trigger_callback( crontab_job* job, void *user_data){
    time_t now; time(&now);

    if(job->next_trigger_time==0){
      if(job->trigger_on_load){
        job->next_trigger_time = now;
      }else{
        job->next_trigger_time = (*job->timer)(job->cron_expr, now);
      }
    }

    if(job->next_trigger_time==CRON_INVALID_INSTANT)
    {
      logger_error("");
    }else{
      logger_debug("diff=%d, now=%d, next_trigger_time=%d", (job->next_trigger_time-now), now, job->next_trigger_time);

      if(now >= job->next_trigger_time){ // 如果当前时间>下一次执行时间, 那么触发执行
          job->last_trigger_time = now;
          job->next_trigger_time = (*job->timer)(job->cron_expr, job->last_trigger_time);

          thpool_add_work(crontab_thread_pool, execute_script_in_thread, (void*)job->action);
          // (*job->runner)(job->id,job->name,job->action,job->payload,user_data);
      }
    }

    return 0;
};

// #endregion crontab implement area
// ////////////////////////////////////////////////////////
// #region misc implement area

struct sqlite_result
{
  JSContext *ctx;

  int rc;
  char* errmsg;

  int columnCount;
  // js array
  JSValue columns;

  int rowCount;
  // js array
  JSValue rows;

  int changeCount;
};

int js_sqlite_callback(void* para, int columnCount, char** columnValue, char** columnName);
static JSValue js_sqlite_exec(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
  JSValue rs = JS_NewObject(ctx);

  const char* sql = JS_ToCString(ctx, argv[0]);
  struct sqlite_result result = {0};
  result.ctx = ctx;
  result.rc = sqlite3_exec(s_context.sqlite, sql, js_sqlite_callback, &result, &result.errmsg);
  JS_FreeCString(ctx, sql);

  JS_SetPropertyStr(ctx, rs, "code", JS_NewInt32(ctx, result.rc));
  if( result.rc == SQLITE_OK ){
    // fprintf(stdout, "Operation done successfully\n");

    if(result.columnCount>0)
    {
      JS_SetPropertyStr(ctx, rs, "columnCount", JS_NewInt32(ctx, result.columnCount));
      JS_SetPropertyStr(ctx, rs, "columns", result.columns);

      JS_SetPropertyStr(ctx, rs, "rowCount", JS_NewInt32(ctx, result.rowCount));
      JS_SetPropertyStr(ctx, rs, "rows", result.rows);

      //  如果有callback function, 逐条调用, 如果没有, 则一次过返回结果
      // JS_IsFunction();
    }else{
      JS_SetPropertyStr(ctx, rs, "changeCount", JS_NewInt32(ctx, result.changeCount));
    }

  }else{
    JS_SetPropertyStr(ctx, rs, "errmsg", JS_NewString(ctx, result.errmsg));

    JS_SetPropertyStr(ctx, rs, "columnCount", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, rs, "rowCount", JS_NewInt32(ctx, 0));

    sqlite3_free(result.errmsg);
  }

  return rs;
}

#define SQLITE_INTEGER  1
#define SQLITE_FLOAT    2
#define SQLITE_TEXT     3
#define SQLITE_BLOB     4
#define SQLITE_NULL     5
static JSValue js_sqlite_exec2(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
  JSValue rs = JS_NewObject(ctx);

  const char* sql = JS_ToCString(ctx, argv[0]);
  struct sqlite_result result = {0};
  result.ctx = ctx;

  int rc;
  sqlite3_stmt* stmt;
  const char *pztail;

  rc=sqlite3_prepare(s_context.sqlite, sql, strlen(sql)+1, &stmt,&pztail);
  if(rc==SQLITE_OK){
    for (size_t i = 1; i < argc; i++)
    {
      JSValueConst value = argv[i];

      if(JS_IsNull(value) || JS_IsUndefined(value)){
        sqlite3_bind_null(stmt,i);
      }else
      if(JS_IsString(value)){
        const char * v = JS_ToCString(ctx, value);
        sqlite3_bind_text(stmt,i,v,-1,SQLITE_STATIC);
        JS_FreeCString(ctx, v);
      }else
      if(JS_IsNumber(value)){
          int64_t v;
          JS_ToInt64(ctx, &v, value);
          double v2;
          JS_ToFloat64(ctx, &v2, value);
        if(v==v2){
          sqlite3_bind_int64(stmt, i, v);
        }else{
          sqlite3_bind_double(stmt, i, v2);
        }
      }else
      if(JS_IsBool(value)){
          sqlite3_bind_int(stmt, i, JS_ToBool(ctx, value));
      // }else{
      //   const char * type_name = JS_ToCString(ctx, type);

      //   if(STR_IS_EQUALS(type_name,"int")){
      //     int32_t v;
      //     JS_Toint(ctx, &v, value);
      //     sqlite3_bind_int(stmt, i, v);
      //   }else
      //   if(STR_IS_EQUALS(type_name,"double")){
      //     double v;
      //     JS_ToFloat64(ctx, &v, value);
      //     sqlite3_bind_double(stmt, i, v);
      //   }else
      //   if(STR_IS_EQUALS(type_name,"bool")){
      //     sqlite3_bind_int(stmt, i, JS_ToBool(ctx, value));
      //   }else
      //   if(STR_IS_EQUALS(type_name,"long")){
      //     int64_t v;
      //     JS_Toint64(ctx, &v, value);
      //     sqlite3_bind_int64(stmt, i, v);
      //   }
        

      }
    }

    int column_count = sqlite3_column_count(stmt); 
    int done = 0;
    while(!done){
      rc = sqlite3_step(stmt);
      switch(rc){
        case SQLITE_ROW :

          if(result.columnCount==0){
            result.columnCount = column_count;
            result.columns = JS_NewArray(ctx);
            result.rows = JS_NewArray(ctx);
            for (size_t i = 0; i < column_count; i++)
            {
              const char* column_name = sqlite3_column_name(stmt,i);
              rc = JS_SetPropertyInt64(ctx, result.columns, i, JS_NewString(ctx, column_name)) ==1 ? 0:1;
              if(rc) goto clean;
            }
            // JSValue json = JS_JSONStringify(ctx, result.columns, JS_NULL, JS_NULL);
            // const char * str = JS_ToCString(ctx, json);
            // logger_info(str);
            // JS_FreeCString(ctx, str);
            // JS_FreeValue(ctx, json);
          }

          JSValue row = JS_NewArray(ctx);
          for (size_t i = 0; i < column_count; i++)
          {
            switch(sqlite3_column_type(stmt , i)){
              case SQLITE_INTEGER:
                rc = JS_SetPropertyInt64(ctx, row, i, JS_NewInt64(ctx, sqlite3_column_int64(stmt , i))) ==1 ? 0:1;
                if(rc) goto clean;
                break;
              case SQLITE_FLOAT:
                rc = JS_SetPropertyInt64(ctx, row, i, JS_NewFloat64(ctx, sqlite3_column_double(stmt , i))) ==1 ? 0:1;
                if(rc) goto clean;
                break;
              case SQLITE_TEXT:
                rc = JS_SetPropertyInt64(ctx, row, i, JS_NewString(ctx, sqlite3_column_text(stmt , i))) ==1 ? 0:1;
                if(rc) goto clean;
                break;
              case SQLITE_BLOB:
              case SQLITE_NULL:
              default:
                rc = JS_SetPropertyInt64(ctx, row, i, JS_NULL) ==1 ? 0:1;
                if(rc) goto clean;
                break;
            }
          }
          rc = JS_SetPropertyInt64(ctx, result.rows, result.rowCount, row) ==1 ? 0:1;
          if(rc) goto clean;
          result.rowCount++;
        break;
        case SQLITE_DONE:
          if(result.columnCount==0 && column_count>0){
            result.columnCount = column_count;
            result.columns = JS_NewArray(ctx);
            for (size_t i = 0; i < column_count; i++)
            {
              const char* column_name = sqlite3_column_name(stmt,i);
              rc = JS_SetPropertyInt64(ctx, result.columns, i, JS_NewString(ctx, column_name)) ==1 ? 0:1;
              if(rc) goto clean;
            }
          }
          result.changeCount = sqlite3_changes(s_context.sqlite);
          done = 1;
        break;
        default:
          done = 2;
        break;
      }
    }

  // boolean:  0:false,1:true  
  clean:
    sqlite3_finalize(stmt);
  }
// typedef struct sqlite3_stmt sqlite3_stmt;
// int sqlite3_prepare(sqlite3, const char, int, sqlite3_stmt, const char);

// int sqlite3_finalize(sqlite3_stmt*);


// int sqlite3_bind_double(sqlite3_stmt*, int, double);
 
// int sqlite3_bind_int(sqlite3_stmt*, int, int);
// int sqlite3_bind_int64(sqlite3_stmt*, int, long long int);
 
// int sqlite3_bind_null(sqlite3_stmt*, int);
 
// int sqlite3_bind_text(sqlite3_stmt, int, const char, int n, void()(void));


//    typeof(str)  //string
//    typeof(num)  //number
//    typeof(obj)  //object
//    typeof(arr)  //object
//    typeof(fn)  //function

// new Date() instanceof Date

// Number.isInteger

// JS_IsBool


// int sqlite3_column_count(sqlite3_stmt*);

// int sqlite3_column_type(sqlite3_stmt*, int iCol);


// const char sqlite3_column_name(sqlite3_stmt, int iCol);


// int sqlite3_column_int(sqlite3_stmt*, int iCol);

// long long int sqlite3_column_int64(sqlite3_stmt*, int iCol);
 
// double sqlite3_column_double(sqlite3_stmt*, int iCol);
 
// const unsigned char sqlite3_column_text(sqlite3_stmt, int iCol);


  JS_FreeCString(ctx, sql);

  JS_SetPropertyStr(ctx, rs, "code", JS_NewInt32(ctx, result.rc));
  if( result.rc == SQLITE_OK ){
    // fprintf(stdout, "Operation done successfully\n");

    if(result.columnCount>0)
    {
      JS_SetPropertyStr(ctx, rs, "columnCount", JS_NewInt32(ctx, result.columnCount));
      JS_SetPropertyStr(ctx, rs, "columns", result.columns);

      JS_SetPropertyStr(ctx, rs, "rowCount", JS_NewInt32(ctx, result.rowCount));
      JS_SetPropertyStr(ctx, rs, "rows", result.rows);

      //  如果有callback function, 逐条调用, 如果没有, 则一次过返回结果
      // JS_IsFunction();
    }else{
      JS_SetPropertyStr(ctx, rs, "changeCount", JS_NewInt32(ctx, result.changeCount));
    }

  }else{
    JS_SetPropertyStr(ctx, rs, "errmsg", JS_NewString(ctx, result.errmsg));

    JS_SetPropertyStr(ctx, rs, "changeCount", JS_NewInt32(ctx, result.changeCount));
    JS_SetPropertyStr(ctx, rs, "columnCount", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, rs, "rowCount", JS_NewInt32(ctx, 0));

    sqlite3_free(result.errmsg);
  }

  return rs;

}

int js_sqlite_callback(void* para, int columnCount, char** columnValue, char** columnName)
{
  int rc = 0;
  struct sqlite_result* rs = para;
  if(rs->columnCount==0){
    rs->columnCount = columnCount;
    rs->columns = JS_NewArray(rs->ctx);
    rs->rows = JS_NewArray(rs->ctx);
    for (size_t i = 0; i < columnCount; i++)
    {
      rc = JS_SetPropertyInt64(rs->ctx, rs->columns, i, JS_NewString(rs->ctx, columnName[i])) ==1 ? 0:1;
      if(rc) goto clean;
    }
  }

  JSValue row = JS_NewArray(rs->ctx);
  for (size_t i = 0; i < columnCount; i++)
  {
    rc = JS_SetPropertyInt64(rs->ctx, row, i, columnValue[i]==NULL ? JS_NULL : JS_NewString(rs->ctx, columnValue[i])) ==1 ? 0:1;
    if(rc) goto clean;
  }
  rc = JS_SetPropertyInt64(rs->ctx, rs->rows, rs->rowCount, row) ==1 ? 0:1;
  if(rc) goto clean;
  rs->rowCount++;
  
  rs->changeCount = sqlite3_changes(s_context.sqlite);

clean:
  // TODO 释放sqlite_result里的引用

  return rc; // 返回0:继续执行, 非0:中断执行
}

// #endregion crontab implement area
// ////////////////////////////////////////////////////////
// #region misc implement area



int load_physical_network_interfaces()
{
  if(nic_map!=NULL){
    return 0;
  }
  nic_map = hashmap_new();

  str_builder_t * sb = str_builder_create();
  const char* cmd = "ls /sys/class/net | egrep -v \"`ls /sys/devices/virtual/net | awk 'BEGIN {print \\\"^&\\\"}{print \\\"|\\\"$0 }' | tr -d '\\n'`\"";
  logger_debug("cmd is %s", cmd);
  execute_cmd_by_popen(cmd, sb);
  size_t last = 0;
  for (size_t i = 0; i < sb->len; i++)
  {
    if(sb->str[i]=='\n'){
      if(i-last>1){
        char* name = calloc(i-last+1, sizeof(char));
        strncpy(name, sb->str+last, i-last);
        struct nic_stat *ns = malloc(sizeof(struct nic_stat));
        ns->name = name;
        ns->n0 = NULL;

        hashmap_put(nic_map, name, ns);
      }
      last = i+1;
    }
  }
  str_builder_destroy(sb);
}

int get_physical_network_interface_io_rate(const char *name, long * rx, long * tx)
{
  struct nic_stat *stat;
  if(hashmap_get(nic_map, name, (void**)&stat) == MAP_OK){
    *rx = stat->read;
    *tx = stat->write;
    return 0;
  }else{
    *rx = -1;
    *tx = -1;
    return -1;
  }
}

struct hashmap_callback_context
{
  JSContext *ctx;
  JSValue *info;
};


int set_physical_nic(JSContext *ctx, JSValue info, struct nic_stat *stat);;;;;;;
static JSValue js_get_physical_network_interface_io_speed(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
  JSValue info = JS_NewObject(ctx);
  
  if(argc==1)
  {
    const char *name = JS_ToCString(ctx, argv[1]);
    struct nic_stat *stat;
    if(hashmap_get(nic_map, name, (void**)&stat) == MAP_OK){
      set_physical_nic(ctx, info, stat);
    }else{
      set_physical_nic(ctx, info, NULL);
    }
    JS_FreeCString(ctx, name);

  }else{
    // all
    struct hashmap_callback_context context;
    context.ctx = ctx;
    context.info = &info;
    hashmap_foreach(nic_map, callback_fn_get_physical_nic, &context);
  }

  return info;
}

int callback_fn_get_physical_nic(any_t map_context, const char* key, any_t value)
{
  struct hashmap_callback_context *context = map_context;
  JSContext *ctx = context->ctx;
  struct nic_stat *stat = value;

  JSValue info = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, *(context->info), key, info);
  set_physical_nic(ctx, info, stat);

  return 0;
}

int set_physical_nic(JSContext *ctx, JSValue info, struct nic_stat *stat)
{
  JS_SetPropertyStr(ctx, info, "read", JS_NewInt64(ctx, stat==NULL ? -1 : stat->read));
  JS_SetPropertyStr(ctx, info, "write", JS_NewInt64(ctx, stat==NULL ? -1 : stat->write));

  // JS_SetPropertyStr(ctx, info, "rx_bytes", JS_NewInt64(ctx, stat==NULL ? -1 : stat->n0->rx_bytes));
  // JS_SetPropertyStr(ctx, info, "tx_bytes", JS_NewInt64(ctx, stat==NULL ? -1 : stat->n0->tx_bytes));
}


int get_file_content(const char* file, struct str_builder *sb);
int callback_fn_update_physical_nic_io_rate(any_t context, const char* key, any_t value);
int update_physical_network_interface_io_rate()
{
  // 循环网卡, 逐个更新速度
  hashmap_foreach(nic_map, callback_fn_update_physical_nic_io_rate, NULL);
}

int callback_fn_update_physical_nic_io_rate(any_t context, const char* key, any_t value)
{
  struct nic_stat *stat = value;

  struct nic *curr = stat->n0 != &stat->n2 ? &stat->n1 : &stat->n2;
  struct nic *next = stat->n0 == &stat->n2 ? &stat->n1 : &stat->n2;

  gettimeofday(&next->ts, NULL );  

  char path[512];
  str_builder_t *sb = str_builder_create();

  sprintf(path, "/sys/class/net/%s/statistics/rx_bytes", key);
  get_file_content(path, sb);
  next->rx_bytes = sb->len > 0 ? atol(sb->str) : 0;

  str_builder_clear(sb);

  sprintf(path, "/sys/class/net/%s/statistics/tx_bytes", key);
  get_file_content(path, sb);
  next->tx_bytes = sb->len > 0 ? atol(sb->str) : 0;

  logger_debug("curr: clock=%ld.%ld,rx_bytes=%ld,tx_bytes=%ld,read=%ld,write=%ld", 
    curr->ts.tv_sec, curr->ts.tv_usec, curr->rx_bytes, curr->tx_bytes, stat->read, stat->write);
  if(stat->n0!=NULL){
    double duration = (double)((next->ts.tv_sec - curr->ts.tv_sec)*1000000 + next->ts.tv_usec - curr->ts.tv_usec) / 1000000;
    stat->read = (long)((double)(next->rx_bytes - curr->rx_bytes) / duration + 0.5);
    stat->write = (long)((double)(next->tx_bytes - curr->tx_bytes) / duration + 0.5);
  }else{
    stat->read = -1;
    stat->write = -1;
  }
  logger_debug("next: clock=%ld.%ld,rx_bytes=%ld,tx_bytes=%ld,read=%ld,write=%ld", 
    next->ts.tv_sec, next->ts.tv_usec, next->rx_bytes, next->tx_bytes, stat->read, stat->write);

  stat->n0 = next; // 切换当前值

  str_builder_destroy(sb);
}

int get_file_content(const char* file, struct str_builder *sb)
{

  char buf_ps[1024];
  FILE *ptr;
  if((ptr=fopen(file, "r"))!=NULL)   
  {
      while(fgets(buf_ps, 1024, ptr)!=NULL)   
      {
        str_builder_add_str(sb, buf_ps, 0);  
      }
      pclose(ptr);   
      ptr = NULL;   
  }   
  else  
  {   
      printf("get_file_content `%s` error\n", file);   
      return -1;
  }

  return 0;
}

// #endregion misc implement area
// ////////////////////////////////////////////////////////////////////////////


#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

void sleep_ms(int milliseconds) // cross-platform sleep function
{
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}
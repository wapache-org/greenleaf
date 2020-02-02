/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

//header for getopt
#include <unistd.h>
//header for getopt_long
#include <getopt.h>

#include "mongoose.h"


static const char *s_http_port = "8000";

static const struct mg_str s_get_method = MG_MK_STR("GET");
static const struct mg_str s_put_method = MG_MK_STR("PUT");
static const struct mg_str s_delele_method = MG_MK_STR("DELETE");

static int s_sig_num = 0;
static struct mg_serve_http_opts s_http_server_opts;

static void *s_db_handle = NULL;
static const char *s_db_path = "sqlite.db";

static void usage(int argc, char* argv[]);
static void parse_options(struct mg_serve_http_opts* opts, int argc, char* argv[]);

static void signal_handler(int sig_num);
static void event_handler(struct mg_connection *nc, int ev, void *ev_data);

static void handle_api_request(struct mg_connection *nc, struct http_message *hm);
static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix);
static int is_equal(const struct mg_str *s1, const struct mg_str *s2);


static int get_user_htpasswd(struct mg_str username, struct mg_str auth_domain, char* out_ha1);
static int check_pass(const char *user, const char *pass);

static const char *s_login_url = "/api/user/login.json";
static const char *s_logout_url = "/api/user/logout.json";
static const char *s_login_user = "username";
static const char *s_login_pass = "password";


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
 * Parses the session cookie and returns a pointer to the session struct
 * or NULL if not found.
 */
static struct session *get_session(struct http_message *hm) {
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

static int get_session_id(struct http_message *hm, char *ssid, size_t len) {
  struct mg_str *cookie_header = mg_get_http_header(hm, "cookie");
  if (cookie_header == NULL) {
      return 0;
  } else {
      return mg_http_parse_header2(cookie_header, SESSION_COOKIE_NAME, &ssid, len);
  }
}

/*
 * Destroys the session state.
 */
static void destroy_session(struct session *s) {
  free(s->user);
  memset(s, 0, sizeof(*s));
}

/*
 * Creates a new session for the user.
 */
static struct session *create_session(const char *user,
                                      const struct http_message *hm) {
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

/*
 * If requested via GET, serves the login page.
 * If requested via POST (form submission), checks password and logs user in.
 */
static void login_handler(struct mg_connection *nc, int ev, void *p) {
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

/*
 * Logs the user out.
 * Removes cookie and any associated session state.
 */
static void logout_handler(struct mg_connection *nc, int ev, void *p) {
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

/* Cleans up sessions that have been idle for too long. */
void check_sessions(void) {
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

/*
 * Password check function.
 * In our example all users have password "password".
 */
static int check_pass(const char *user, const char *pass) {
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

static int send_cookie_auth_request(struct mg_connection *nc, char* message)
{
    mg_printf(nc, "HTTP/1.0 403 Unauthorized\r\n\r\n%s\r\n", message == NULL ? "Unauthorized." : message);
    return 1;
}

static 
int get_user_htpasswd(struct mg_str username, struct mg_str auth_domain, char* out_ha1)
{
    // out_ha1 is char[128]
    // username/password: admin/admin
    strcpy(out_ha1,"609e3552947b5949c2451a072a2963e1");
    return 1; // found
}

int main(int argc, char *argv[]) {

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

    /* Run event loop until signal is received */
    printf("Starting RESTful server on port %s\n", s_http_port);
    while (s_sig_num == 0) {
        mg_mgr_poll(&mgr, 1000);
    }

    /* Cleanup */
    mg_mgr_free(&mgr);
    // db_close(&s_db_handle);

    printf("Exiting on signal %d\n", s_sig_num);

    return 0;
}

static void signal_handler(int sig_num) {
    signal(sig_num, signal_handler);
    s_sig_num = sig_num;
}

static const struct mg_str api_prefix = MG_MK_STR("/api/");
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

static void handle_api_request(struct mg_connection *nc, struct http_message *hm)
{
    // 需要改成用hashmap
    char* path = hm->uri.p+api_prefix.len-1;
    int length = hm->uri.len-api_prefix.len-4; // +1-strlen(".json")
    if (strncmp(path, "/services/host", length)==0)
    {
        // 从数据库读取节点的信息
        // 然后连接到host, 获取更进一步的信息
    }
    

    mg_serve_http(nc, hm, s_http_server_opts);
}

static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix) {
    return uri->len > prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static int is_equal(const struct mg_str *s1, const struct mg_str *s2) {
    return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
}


// 短选项字符串, 一个字母表示一个短参数, 如果字母后带有冒号, 表示这个参数必须带有参数
// 建议按字母顺序编写
static char* short_opts = "a:d:hlp:r:";
// 长选项字符串, {长选项名字, 0:没有参数|1:有参数|2:参数可选, flags, 短选项名字}
// 建议按长选项字母顺序编写
static const struct option long_options[] = {
		{"auth-domain",1,NULL,'a'},
		{"database",1,NULL,'d'},
		{"enable-directory-listing",0,NULL,'l'},
		{"help",0,NULL,'h'},
		{"port",1,NULL,'p'},
		{"root",1,NULL,'r'}
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
  printf("    [-%s, --%s]     %s\n", "r","root","web server root directory, default is `html`.");
  printf("    [-%s, --%s]     %s\n", "l","enable-directory-listing","if cannot find index file, list directory files, default is no.");
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

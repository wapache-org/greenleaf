// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved

//header for getopt
#include <unistd.h>
//header for getopt_long
#include <getopt.h>

#include "mongoose.h"

static void usage(int argc, char* argv[]);
void parse_options(struct mg_serve_http_opts* opts, int argc, char* argv[]);


static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
  }
}

int main(int argc, char* argv[]) {

  parse_options(&s_http_server_opts, argc,argv);

  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  printf("Starting web server on port %s\n", s_http_port);
  nc = mg_bind(&mgr, s_http_port, ev_handler);
  if (nc == NULL) {
    printf("Failed to create listener\n");
    return 1;
  }

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}

// 短选项字符串, 一个字母表示一个短参数, 如果字母后带有冒号, 表示这个参数必须带有参数
// 建议按字母顺序编写
static char* short_opts = "hlp:r:";
// 长选项字符串, {长选项名字, 0:没有参数|1:有参数|2:参数可选, flags, 短选项名字}
// 建议按长选项字母顺序编写
static const struct option long_options[] = {
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
  printf("    [-%s, --%s]     %s\n", "p","poot","web server bingding port, default is 8000.");
  printf("    [-%s, --%s]     %s\n", "r","root","web server root directory, default is current work directory.");
  printf("    [-%s, --%s]     %s\n", "l","enable-directory-listing","if cannot find index file, list directory files, default is no.");
}
// 解析选项
void parse_options(struct mg_serve_http_opts* opts, int argc, char* argv[])
{

  // 先设置默认值
  opts->enable_directory_listing = "no";

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

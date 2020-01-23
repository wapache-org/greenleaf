/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "mongoose.h"

static const char *s_http_port = "8000";

static sig_atomic_t s_signal_received = 0;
static struct mg_serve_http_opts s_http_server_opts;

static void signal_handler(int sig_num);
static void event_handler(struct mg_connection *nc, int ev, void *ev_data);

static int is_websocket(const struct mg_connection *nc);
static void broadcast(struct mg_connection *nc, const struct mg_str msg);

int main(int argc, char* argv[]) 
{
  // 检查启动参数
  if(argc<=1){
    printf("usage: %s web-root-path\n", argv[0]);
    return -1;
  }

  // 注册系统信号处理器
  signal(SIGTERM, signal_handler); // 请求终止进程, kill命令缺省发送
  signal(SIGINT, signal_handler);  // 请求中断进程, CTRL+C触发

  // 设置文件IO缓冲方式, _IOLBF: io line buffer, _IONBF: io no buffer, _IOFBF: io full buffer
  setvbuf(stdout, NULL, _IOLBF, 0); // 设置标准输出为行缓存
  setvbuf(stderr, NULL, _IOLBF, 0); // 设置标准错误为行缓存

  // 设置HTTP请求的处理选项
  char* path = argv[1];
  s_http_server_opts.document_root = path;
  // s_http_server_opts.enable_directory_listing = "no";
  
  // 启动服务器
  struct mg_mgr mgr;
  struct mg_connection *nc;
  
  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, event_handler);
  mg_set_protocol_http_websocket(nc);
  printf("start websocket server on port %s\n", s_http_port);

  // 事件处理主循环
  // 当接收到信号后退出循环
  while (s_signal_received == 0) {
    mg_mgr_poll(&mgr, 200);
  }

  // 释放资源
  mg_mgr_free(&mgr);

  return 0;
}

static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);  // Reinstantiate signal handler
  s_signal_received = sig_num;      // 记录信号值, 以便main函数中的主循环能在接收到信号后退出循环
}

static void event_handler(struct mg_connection *nc, int event, void *event_data) {
  switch (event) {
    case MG_EV_HTTP_REQUEST: { // 接收到HTTP请求
      mg_serve_http(nc, (struct http_message *) event_data, s_http_server_opts);
      break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: { // WEBSOCKET握手完成, 建立连接成功
      /* New websocket connection. Tell everybody. */
      broadcast(nc, mg_mk_str("++ joined"));
      break;
    }
    case MG_EV_WEBSOCKET_FRAME: { // 接收到客户端发过来的消息
      struct websocket_message *wm = (struct websocket_message *) event_data;
      /* New websocket message. Tell everybody. */
      struct mg_str d = {(char *) wm->data, wm->size};
      broadcast(nc, d);
      break;
    }
    case MG_EV_CLOSE: {
      /* Disconnect. Tell everybody. */
      if (is_websocket(nc)) {
        broadcast(nc, mg_mk_str("-- left"));
      }
      break;
    }
  }
}

// 广播消息
static void broadcast(struct mg_connection *nc, const struct mg_str msg) 
{
  char addr[32];
  mg_sock_addr_to_str(
    &nc->sa, 
    addr, sizeof(addr),
    MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT
  );

  // 消息的最大长度是500...
  char buf[500];
  snprintf(buf, sizeof(buf), "%s %.*s", addr, (int) msg.len, msg.p);
  printf("%s\n", buf); /* Local echo. */

  // 遍历所有客户端连接
  struct mg_connection *c;
  for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
    // if (c == nc) continue; /* Don't send to the sender. */
    // 发送消息
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, buf, strlen(buf));
  }
}

// 判断连接是否是websocket连接
static int is_websocket(const struct mg_connection *nc) {
  return nc->flags & MG_F_IS_WEBSOCKET;
}
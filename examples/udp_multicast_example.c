// Copyright (c) 2016 Cesanta Software Limited
// All rights reserved
//
// This software is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this software under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this software under a commercial
// license, as set out in <https://www.cesanta.com/license>.
//
// This demo shows how to configure a mongoose UDP connection to receive
// multicast traffic.

#include <netinet/in.h>

#include "mongoose.h"

#define DEFAULT_GROUP "224.0.22.1"
#define DEFAULT_PORT "1234"

static void usage(char **argv) {
  fprintf(stderr, 
    "%s: [-h] [-i address_of_mcast_interface] [ -g %s ] [ -p %s ] [ -c heartbeat-content]\n",
    argv[0], DEFAULT_GROUP, DEFAULT_PORT
  );
  exit(1);
}

// 事件管理器
struct mg_mgr mgr;
// 组播"连接"
struct mg_connection *group_receiver;
struct mg_connection *group_sender;
// 心跳内容
static char* heartbeat_content = "heartbeat.";


// 事件处理器
static void ev_handler(struct mg_connection *nc, int ev, void *p);
// 发送组播信息
static void multicast(void * buf, size_t len);
// 设置发送组播信息时间
static void set_multicast_timer();

int main(int argc, char **argv) {

  /* Parse command line arguments */
  const char *interface = NULL;
  const char *mcast_group = DEFAULT_GROUP;
  const char *port = DEFAULT_PORT;
  for (int i = 1; i < argc; i++) {
    // IP address of the interface where to join a multicast group.
    if (strcmp(argv[i], "-i") == 0) {
      interface = argv[++i];
    } else if (strcmp(argv[i], "-g") == 0) {
      mcast_group = argv[++i];
    } else if (strcmp(argv[i], "-p") == 0) {
      port = argv[++i];
    } else if (strcmp(argv[i], "-c") == 0) {
      heartbeat_content = argv[++i];
    }else{
      usage(argv);
      exit(1);
    }
  }

  mg_mgr_init(&mgr, NULL);
  {
    char listen[256];
    snprintf(listen, sizeof(listen), "udp://%s", port);

    group_receiver = mg_bind(&mgr, listen, ev_handler);
    if (group_receiver == NULL) {
      perror("cannot bind\n");
      exit(1);
    }

    char group_address[256];
    snprintf(group_address, sizeof(group_address), "udp://%s:%s", mcast_group, port);
    group_sender = mg_connect(&mgr, group_address, ev_handler);
    if (group_sender == NULL) {
      perror("cannot connect multicast address\n");
      exit(1);
    }

  }

  /* 允许同一个程序的多个运行实例绑定到同一个端口
  Enable SO_REUSEADDR to allow multiple instances of this application to receive copies of the multicast datagrams. */
  // {
  //     int reuse = 1;
  //     if(setsockopt(group_connection->sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
  //     {
  //         perror("Setting SO_REUSEADDR error");
  //         exit(1);
  //     }
  //     else
  //         printf("Setting SO_REUSEADDR...OK.\n");
  // }

  struct ip_mreq group;
  group.imr_multiaddr.s_addr = inet_addr(mcast_group);
  group.imr_interface.s_addr = interface==NULL ? htonl(INADDR_ANY) : inet_addr(interface);
  if (setsockopt(group_receiver->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &group, sizeof(group)) < 0) {
    perror("Adding multicast group error");
    exit(1);
  }

  // send heartbeat every 5 seconds
  set_multicast_timer();

  printf("Starting multicast server on port %s listening to group %s\n", port, mcast_group);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  (void) p;
  switch (ev) {
    case MG_EV_TIMER: // 定时发送心跳
      if(group_sender == nc) {
        multicast(heartbeat_content, strlen(heartbeat_content));
        set_multicast_timer();
      }
    break;
    case MG_EV_SEND: { // 心跳发送成功
      if(group_sender == nc) {
        const char *peer = inet_ntoa(nc->sa.sin.sin_addr);
        uint16_t port = ntohs(nc->sa.sin.sin_port);
        printf("%f Heartbeat Sended to %s from port %d\n", mg_time(), peer, port);
      }
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
    // case MG_EV_POLL: {
    //   if(group_receiver != nc && group_sender != nc){
    //     const char *peer = inet_ntoa(nc->sa.sin.sin_addr);
    //     printf("poll: nc->sa: %s %d\n", peer, ntohs(nc->sa.sin.sin_port));
    //   }
    // }
    // break;
    default:
      break;
  }
}

static void multicast(void * buf, size_t len) {
  mg_send(group_sender, buf, len);
}

static void set_multicast_timer() {
  mg_set_timer(group_sender, mg_time() + 5);
}



```c

// # mongoose事件循环研究

// 事件循环代码:
{
    struct mg_mgr mgr;
    mg_mgr_init(&mgr, NULL);
 
	... ...
	
    while (s_sig_num == 0) {
        mg_mgr_poll(&mgr, 1000); // 重点是这里
    }

    mg_mgr_free(&mgr);
}

// ## 涉及到的数据结构

/*
 * Mongoose event manager.
 */
struct mg_mgr {
  struct mg_connection *active_connections;
#if MG_ENABLE_HEXDUMP
  const char *hexdump_file; /* Debug hexdump file path */
#endif
#if MG_ENABLE_BROADCAST
  sock_t ctl[2]; /* Socketpair for mg_broadcast() */
	/*
		mg_broadcast() 往这个socket pair 写数据
		mg_mgr_poll() 从这个socket pair 读数据, 然后调用回调函数, Thus the callback function executes in event manager thread.
	*/
#endif
  void *user_data; /* User data */
  int num_ifaces; /* 网卡数量, ifaces数组的长度 */
  int num_calls; /* mg_call函数的非MG_EV_POLL的调用数 */
  struct mg_iface* *ifaces; /* network interfaces , 可以认为就是网卡 */
  const char *nameserver;   /* DNS server to use */
};

struct mg_iface {
  struct mg_mgr *mgr;
  void *data; /* Implementation-specific data */
  const struct mg_iface_vtable *vtable;
};


struct mg_iface_vtable {
  void (*init)(struct mg_iface *iface);
  void (*free)(struct mg_iface *iface);
  void (*add_conn)(struct mg_connection *nc);
  void (*remove_conn)(struct mg_connection *nc);
  time_t (*poll)(struct mg_iface *iface, int timeout_ms);

  /* Set up a listening TCP socket on a given address. rv = 0 -> ok. */
  int (*listen_tcp)(struct mg_connection *nc, union socket_address *sa);
  /* Request that a "listening" UDP socket be created. */
  int (*listen_udp)(struct mg_connection *nc, union socket_address *sa);

  /* Request that a TCP connection is made to the specified address. */
  void (*connect_tcp)(struct mg_connection *nc, const union socket_address *sa);
  /* Open a UDP socket. Doesn't actually connect anything. */
  void (*connect_udp)(struct mg_connection *nc);

  /* Send functions for TCP and UDP. Sent data is copied before return. */
  int (*tcp_send)(struct mg_connection *nc, const void *buf, size_t len);
  int (*udp_send)(struct mg_connection *nc, const void *buf, size_t len);

  int (*tcp_recv)(struct mg_connection *nc, void *buf, size_t len);
  int (*udp_recv)(struct mg_connection *nc, void *buf, size_t len,
                  union socket_address *sa, size_t *sa_len);

  /* Perform interface-related connection initialization. Return 1 on ok. */
  int (*create_conn)(struct mg_connection *nc);
  /* Perform interface-related cleanup on connection before destruction. */
  void (*destroy_conn)(struct mg_connection *nc);

  /* Associate a socket to a connection. */
  void (*sock_set)(struct mg_connection *nc, sock_t sock);

  /* Put connection's address into *sa, local (remote = 0) or remote. */
  void (*get_conn_addr)(struct mg_connection *nc, int remote,
                        union socket_address *sa);
};

// ## 涉及到的函数

// 在主线程里, 循环调用mg_mgr_poll, 
int mg_mgr_poll(struct mg_mgr *m, int timeout_ms) {
  int i, num_calls_before = m->num_calls;

  for (i = 0; i < m->num_ifaces; i++) {
    m->ifaces[i]->vtable->poll(m->ifaces[i], timeout_ms);
  }

  return (m->num_calls - num_calls_before);
}

static time_t mg_null_if_poll(struct mg_iface *iface, int timeout_ms) {
  struct mg_mgr *mgr = iface->mgr;
  struct mg_connection *nc, *tmp;
  double now = mg_time();
  /* We basically just run timers and poll. */
  for (nc = mgr->active_connections; nc != NULL; nc = tmp) { // 遍历每一个网卡的连接, 会触发一次mg_timer
    tmp = nc->next;
    mg_if_poll(nc, now);
  }
  (void) timeout_ms;
  return (time_t) now;
}


int mg_if_poll(struct mg_connection *nc, double now) {
  if (nc->flags & MG_F_CLOSE_IMMEDIATELY) {
    mg_close_conn(nc);
    return 0;
  } else if (nc->flags & MG_F_SEND_AND_CLOSE) {
    if (nc->send_mbuf.len == 0) {
      nc->flags |= MG_F_CLOSE_IMMEDIATELY;
      mg_close_conn(nc);
      return 0;
    }
  } else if (nc->flags & MG_F_RECV_AND_CLOSE) {
    mg_close_conn(nc);
    return 0;
  }
#if MG_ENABLE_SSL
  if ((nc->flags & (MG_F_SSL | MG_F_LISTENING | MG_F_CONNECTING)) == MG_F_SSL) {
    /* SSL library may have data to be delivered to the app in its buffers,
     * drain them. */
    int recved = 0;
    do {
      if (nc->flags & (MG_F_WANT_READ | MG_F_WANT_WRITE)) break;
      if (recv_avail_size(nc, MG_TCP_IO_SIZE) <= 0) break;
      recved = mg_do_recv(nc);
    } while (recved > 0);
  }
#endif /* MG_ENABLE_SSL */
  mg_timer(nc, now);
  {
    time_t now_t = (time_t) now;
    mg_call(nc, NULL, nc->user_data, MG_EV_POLL, &now_t);
  }
  return 1;
}

MG_INTERNAL void mg_timer(struct mg_connection *c, double now) {
  if (c->ev_timer_time > 0 && now >= c->ev_timer_time) { // 如果timer时间到了, 则触发timer事件
    double old_value = c->ev_timer_time;
    c->ev_timer_time = 0;
    mg_call(c, NULL, c->user_data, MG_EV_TIMER, &old_value);
  }
}

MG_INTERNAL void mg_call(struct mg_connection *nc,
                         mg_event_handler_t ev_handler, void *user_data, int ev,
                         void *ev_data) {
  if (ev_handler == NULL) {
    /*
     * If protocol handler is specified, call it. Otherwise, call user-specified
     * event handler.
     */
    ev_handler = nc->proto_handler ? nc->proto_handler : nc->handler;
  }
  if (ev != MG_EV_POLL) {
    DBG(("%p %s ev=%d ev_data=%p flags=0x%lx rmbl=%d smbl=%d", nc,
         ev_handler == nc->handler ? "user" : "proto", ev, ev_data, nc->flags,
         (int) nc->recv_mbuf.len, (int) nc->send_mbuf.len));
  }

#if !defined(NO_LIBC) && MG_ENABLE_HEXDUMP
  if (nc->mgr->hexdump_file != NULL && ev != MG_EV_POLL && ev != MG_EV_RECV &&
      ev != MG_EV_SEND /* handled separately */) {
    mg_hexdump_connection(nc, nc->mgr->hexdump_file, NULL, 0, ev);
  }
#endif
  if (ev_handler != NULL) {
    unsigned long flags_before = nc->flags;
    ev_handler(nc, ev, ev_data MG_UD_ARG(user_data));
    /* Prevent user handler from fiddling with system flags. */
    if (ev_handler == nc->handler && nc->flags != flags_before) {
      nc->flags = (flags_before & ~_MG_CALLBACK_MODIFIABLE_FLAGS_MASK) |
                  (nc->flags & _MG_CALLBACK_MODIFIABLE_FLAGS_MASK);
    }
  }
  if (ev != MG_EV_POLL) nc->mgr->num_calls++;
  if (ev != MG_EV_POLL) {
    DBG(("%p after %s flags=0x%lx rmbl=%d smbl=%d", nc,
         ev_handler == nc->handler ? "user" : "proto", nc->flags,
         (int) nc->recv_mbuf.len, (int) nc->send_mbuf.len));
  }
#if !MG_ENABLE_CALLBACK_USERDATA
  (void) user_data;
#endif
}

// ====================================================

time_t mg_socket_if_poll(struct mg_iface *iface, int timeout_ms) {
  struct mg_mgr *mgr = iface->mgr;
  double now = mg_time();
  double min_timer;
  struct mg_connection *nc, *tmp;
  struct timeval tv;
  fd_set read_set, write_set, err_set;
  sock_t max_fd = INVALID_SOCKET;
  int num_fds, num_ev, num_timers = 0;
#ifdef __unix__
  int try_dup = 1;
#endif

  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_ZERO(&err_set);
#if MG_ENABLE_BROADCAST
  mg_add_to_set(mgr->ctl[1], &read_set, &max_fd); /* 把mgr->ctl[1]加入到需要检查可读事件的fd集合 */
#endif

  /*
   * Note: it is ok to have connections with sock == INVALID_SOCKET in the list,
   * e.g. timer-only "connections".
   */
  min_timer = 0;
  for (nc = mgr->active_connections, num_fds = 0; nc != NULL; nc = tmp) { /* 遍历所有连接 */
    tmp = nc->next;

    if (nc->sock != INVALID_SOCKET) {
      num_fds++;

#ifdef __unix__
      /* A hack to make sure all our file descriptos fit into FD_SETSIZE. */
      if (nc->sock >= (sock_t) FD_SETSIZE && try_dup) {
        int new_sock = dup(nc->sock);
        if (new_sock >= 0) {
          if (new_sock < (sock_t) FD_SETSIZE) {
            closesocket(nc->sock);
            DBG(("new sock %d -> %d", nc->sock, new_sock));
            nc->sock = new_sock;
          } else {
            closesocket(new_sock);
            DBG(("new sock is still larger than FD_SETSIZE, disregard"));
            try_dup = 0;
          }
        } else {
          try_dup = 0;
        }
      }
#endif

      if (nc->recv_mbuf.len < nc->recv_mbuf_limit &&
          (!(nc->flags & MG_F_UDP) || nc->listener == NULL)) {
        mg_add_to_set(nc->sock, &read_set, &max_fd); /* 缓冲区还有空间的, 加入到需要检查可读事件的fd集合 */
      }

      if (((nc->flags & MG_F_CONNECTING) && !(nc->flags & MG_F_WANT_READ)) ||
          (nc->send_mbuf.len > 0 && !(nc->flags & MG_F_CONNECTING))) {
        mg_add_to_set(nc->sock, &write_set, &max_fd); /* 写缓冲区有数据的, 加入到需要检查可写事件的fd集合 */
        mg_add_to_set(nc->sock, &err_set, &max_fd); /* 写缓冲区有数据的, 加入到需要检查错误事件的fd集合 */
      }
    }

    if (nc->ev_timer_time > 0) {
      if (num_timers == 0 || nc->ev_timer_time < min_timer) {
        min_timer = nc->ev_timer_time;
      }
      num_timers++;
    }
  }

  /*
   * If there is a timer to be fired earlier than the requested timeout,
   * adjust the timeout.
   */
  if (num_timers > 0) {
    double timer_timeout_ms = (min_timer - mg_time()) * 1000 + 1 /* rounding */;
    if (timer_timeout_ms < timeout_ms) {
      timeout_ms = (int) timer_timeout_ms;
    }
  }
  if (timeout_ms < 0) timeout_ms = 0;

  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  num_ev = select((int) max_fd + 1, &read_set, &write_set, &err_set, &tv); /* mg_poll的核心, 就是select系统调用!! */
  now = mg_time();
#if 0
  DBG(("select @ %ld num_ev=%d of %d, timeout=%d", (long) now, num_ev, num_fds,
       timeout_ms));
#endif

#if MG_ENABLE_BROADCAST
  if (num_ev > 0 && mgr->ctl[1] != INVALID_SOCKET &&
      FD_ISSET(mgr->ctl[1], &read_set)) {
    mg_mgr_handle_ctl_sock(mgr); /* 广播sock(mg内部管理sock)检测到有可读事件, 处理之 */
  }
#endif

  for (nc = mgr->active_connections; nc != NULL; nc = tmp) {
    int fd_flags = 0;
    if (nc->sock != INVALID_SOCKET) {
      if (num_ev > 0) {
        fd_flags = (FD_ISSET(nc->sock, &read_set) &&
                            (!(nc->flags & MG_F_UDP) || nc->listener == NULL)
                        ? _MG_F_FD_CAN_READ
                        : 0) |
                   (FD_ISSET(nc->sock, &write_set) ? _MG_F_FD_CAN_WRITE : 0) |
                   (FD_ISSET(nc->sock, &err_set) ? _MG_F_FD_ERROR : 0);
      }
#if MG_LWIP
      /* With LWIP socket emulation layer, we don't get write events for UDP */
      if ((nc->flags & MG_F_UDP) && nc->listener == NULL) {
        fd_flags |= _MG_F_FD_CAN_WRITE;
      }
#endif
    }
    tmp = nc->next;
    mg_mgr_handle_conn(nc, fd_flags, now); /* 遍历客户端的连接,处理之, 不管有没有检测到可读或可写或出错事件 */
  }

  return (time_t) now;
}


void mg_add_to_set(sock_t sock, fd_set *set, sock_t *max_fd) {
  if (sock != INVALID_SOCKET
#ifdef __unix__
      && sock < (sock_t) FD_SETSIZE
#endif
      ) {
    FD_SET(sock, set);
    if (*max_fd == INVALID_SOCKET || sock > *max_fd) {
      *max_fd = sock;
    }
  }
}

#if MG_ENABLE_BROADCAST
static void mg_mgr_handle_ctl_sock(struct mg_mgr *mgr) {
  struct ctl_msg ctl_msg;
  int len =
      (int) MG_RECV_FUNC(mgr->ctl[1], (char *) &ctl_msg, sizeof(ctl_msg), 0);
  size_t dummy = MG_SEND_FUNC(mgr->ctl[1], ctl_msg.message, 1, 0);
  DBG(("read %d from ctl socket", len));
  (void) dummy; /* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=25509 */
  if (len >= (int) sizeof(ctl_msg.callback) && ctl_msg.callback != NULL) {
    struct mg_connection *nc;
    for (nc = mg_next(mgr, NULL); nc != NULL; nc = mg_next(mgr, nc)) { /* 所有连接都被调用一次, 以达到广播的效果... */
      ctl_msg.callback(nc, MG_EV_POLL,
                       ctl_msg.message MG_UD_ARG(nc->user_data));
    }
  }
}
#endif

void mg_mgr_handle_conn(struct mg_connection *nc, int fd_flags, double now) {
  int worth_logging =
      fd_flags != 0 || (nc->flags & (MG_F_WANT_READ | MG_F_WANT_WRITE));
  if (worth_logging) {
    DBG(("%p fd=%d fd_flags=%d nc_flags=0x%lx rmbl=%d smbl=%d", nc, nc->sock,
         fd_flags, nc->flags, (int) nc->recv_mbuf.len,
         (int) nc->send_mbuf.len));
  }

  if (!mg_if_poll(nc, now)) return;

  if (nc->flags & MG_F_CONNECTING) {
    if (fd_flags != 0) {
      int err = 0;
#if !defined(MG_ESP8266)
      if (!(nc->flags & MG_F_UDP)) {
        socklen_t len = sizeof(err);
        int ret =
            getsockopt(nc->sock, SOL_SOCKET, SO_ERROR, (char *) &err, &len);
        if (ret != 0) {
          err = 1;
        } else if (err == EAGAIN || err == EWOULDBLOCK) {
          err = 0;
        }
      }
#else
      /*
       * On ESP8266 we use blocking connect.
       */
      err = nc->err;
#endif
      mg_if_connect_cb(nc, err); /* 客户端建立连接成功, 调用回调 */
    } else if (nc->err != 0) {
      mg_if_connect_cb(nc, nc->err); /* 客户端建立连接出错, 调用回调 */
    }
  }

  if (fd_flags & _MG_F_FD_CAN_READ) {
    if (nc->flags & MG_F_UDP) {
      mg_if_can_recv_cb(nc); /* 连接有可读数据, 调用回调 */
    } else {
      if (nc->flags & MG_F_LISTENING) {
        /*
         * We're not looping here, and accepting just one connection at
         * a time. The reason is that eCos does not respect non-blocking
         * flag on a listening socket and hangs in a loop.
         */
        mg_accept_conn(nc); /* 服务端接收到新连接, 处理之 */
      } else {
        mg_if_can_recv_cb(nc); /* 连接有可写数据, 调用回调 */
      }
    }
  }

  if (fd_flags & _MG_F_FD_CAN_WRITE) mg_if_can_send_cb(nc);

  if (worth_logging) {
    DBG(("%p after fd=%d nc_flags=0x%lx rmbl=%d smbl=%d", nc, nc->sock,
         nc->flags, (int) nc->recv_mbuf.len, (int) nc->send_mbuf.len));
  }
}

```

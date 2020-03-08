Introduction
================

Mongoose is a networking library written in C.
Mongoose是一个C写的网络通信库.

It is a swiss army knife for embedded network programming.
它是嵌入式网络编程里的瑞士军刀.

It implements event-driven non-blocking APIs for TCP, UDP, HTTP,
WebSocket, CoAP, MQTT for client and server mode.
它实现了基于事件驱动非阻塞的客户端和服务器端APIs, 协议囊括TCP, UDP, HTTP,
WebSocket, CoAP, MQTT.

Features include:

- Cross-platform: works on Linux/UNIX, MacOS, QNX, eCos, Windows, Android,
  iPhone, FreeRTOS
- Native support for PicoTCP embedded TCP/IP stack,
  LWIP embedded TCP/IP stack
- Works on a variety of embedded boards: TI CC3200, TI MSP430, STM32, ESP8266;
  on all Linux-based boards like Raspberry PI, BeagleBone, etc
- Single-threaded, asynchronous, non-blocking core with simple event-based API
- Built-in protocols:
  - plain TCP, plain UDP, SSL/TLS (one-way or two-way), client and server
  - HTTP client and server
  - WebSocket client and server
  - MQTT client and server
  - CoAP client and server
  - DNS client and server
  - asynchronous DNS resolver
- Tiny static and run-time footprint
- Source code is both ISO C and ISO C++ compliant
- Very easy to integrate: just copy
  mongoose.c and
  mongoose.h
  files to your build tree


# Connection flags 连接选项

Each connection has a `flags` bit field. Some flags are set by Mongoose, for
example if a user creates an outbound UDP connection using a `udp://1.2.3.4:5678`
address, Mongoose is going to set a `MG_F_UDP` flag for that connection. Other
flags are meant to be set only by the user event handler to tell Mongoose how to
behave.  Below is a list of connection flags that are meant to be set by event
handlers:
每一个MG连接都可以通过`flags`参数设置连接属性, 有一些是由MG来设置的, 另外一些这是由事件处理器来设置,
事件处理器可以设置的选项有:

* `MG_F_SEND_AND_CLOSE` tells Mongoose that all data has been appended
  to the `send_mbuf`. As soon as Mongoose sends it to the socket, the
  connection will be closed.
  告诉MG所有数据已经添加到发送缓存了, 只要发送完这些数据就可以关闭连接.
* `MG_F_BUFFER_BUT_DONT_SEND` tells Mongoose to append data to the `send_mbuf`
  but hold on sending it, because the data will be modified later and then will
  be sent by clearing the `MG_F_BUFFER_BUT_DONT_SEND` flag.
  告诉MG只是添加数据到缓冲区,不要将数据发送出去. 因为数据可能会在后续处理中做修改.
* `MG_F_CLOSE_IMMEDIATELY` tells Mongoose to close the connection immediately,
  usually after an error.
  通常在出现错误的时候使用, 告诉MG立刻断开连接.
* `MG_F_USER_1`, `MG_F_USER_2`, `MG_F_USER_3`, `MG_F_USER_4` could be used by a
  developer to store an application-specific state.
  这几个标记是预留给应用程序定义自己的一些特有的控制选项的

Flags below are set by Mongoose:
以下选项是由MG内部自己设置的, 一般开发者不需要关心:

* `MG_F_SSL_HANDSHAKE_DONE` SSL only, set when SSL handshake is done.
* `MG_F_CONNECTING` set when the connection is in connecting state after
  `mg_connect()` call but connect did not finish yet.
* `MG_F_LISTENING` set for all listening connections.
* `MG_F_UDP` set if the connection is UDP.
* `MG_F_IS_WEBSOCKET` set if the connection is a WebSocket connection.
* `MG_F_WEBSOCKET_NO_DEFRAG` should be set by a user if the user wants to switch
  off automatic WebSocket frame defragmentation.


# Event handler function

Each connection has an event handler function associated with it. That function
must be implemented by the user. Event handler is the key element of the Mongoose
application, since it defines the application's behaviour. This is what an event
handler function looks like:
每一个连接都有一个对应的事件处理函数, 用户通过实现这个函数来实现应用功能, 这个函数看起来是这样的:

```c
static void event_handler(
    struct mg_connection *nc, 
    int event,        // 事件的编号
    void *event_data) // 不同的事件携带的数据各不相同, 通过头文件可以查看到每种事件的消息格式定义
{
    // 通过switch(事件编码)来对不同的事件做出响应/处理
  switch (event) {
    /* Event handler code that defines behavior of the connection */
    ...
  }
}
```

NOTE: `struct mg_connection` has `void *user_data` which is a placeholder for
application-specific data. Mongoose does not use that pointer. Event handler
can store any kind of information there.
注意: 每个连接都预留了一个`user_data`指针, 用户可以用它来携带应用特有的数据, MG内部不会用到这个指针.用户在事件处理函数里可以用它来保存任何信息


# Events

Mongoose accepts incoming connections, reads and writes data and calls
specified event handlers for each connection when appropriate. A typical event
sequence is this:

- For an outbound connection: 出站连接:
  `MG_EV_CONNECT` -> (`MG_EV_RECV`, `MG_EV_SEND`, `MG_EV_POLL` ...) -> `MG_EV_CLOSE`
- For an inbound connection: 入站连接:
  `MG_EV_ACCEPT` ->  (`MG_EV_RECV`, `MG_EV_SEND`, `MG_EV_POLL` ...) -> `MG_EV_CLOSE`


Below is a list of core events triggered by Mongoose (note that each protocol
triggers protocol-specific events in addition to the core ones):

- `MG_EV_ACCEPT`: sent when a new server connection is accepted by a listening
  connection. `void *ev_data` is `union socket_address` of the remote peer.

- `MG_EV_CONNECT`: sent when a new outbound connection created by `mg_connect()`
  either failed or succeeded. `void *ev_data` is `int *success`.  If `success`
  is 0, then the connection has been established, otherwise it contains an error code.
  See `mg_connect_opt()` function for code example.

- `MG_EV_RECV`: New data is received and appended to the end of `recv_mbuf`.
  `void *ev_data` is `int *num_received_bytes`. Typically, event handler should
  check received data in `nc->recv_mbuf`, discard processed data by calling
  `mbuf_remove()`, set connection flags `nc->flags` if necessary (see `struct
  mg_connection`) and write data the remote peer by output functions like
  `mg_send()`.

  **WARNING**: Mongoose uses `realloc()` to expand the receive buffer. It is
  the user's responsibility to discard processed data from the beginning of the receive
  buffer, note the `mbuf_remove()` call in the example above.

- `MG_EV_SEND`: Mongoose has written data to the remote peer and discarded
  written data from the `mg_connection::send_mbuf`. `void *ev_data` is `int
  *num_sent_bytes`.

  **NOTE**: Mongoose output functions only append data to the
  `mg_connection::send_mbuf`. They do not do any socket writes. An actual IO
  is done by `mg_mgr_poll()`. An `MG_EV_SEND` event is just a notification about
  an IO has been done.

- `MG_EV_POLL`: Sent to all connections on each invocation of `mg_mgr_poll()`.
  This event could be used to do any housekeeping, for example check whether a
  certain timeout has expired and closes the connection or send heartbeat
  message, etc.这个事件可以用来做一些管理性质的工作, 譬如关闭一些超时的连接,定期发送心跳等等.

- `MG_EV_TIMER`: Sent to the connection if `mg_set_timer()` was called.

# Memory buffers


Each connection has a send and receive buffer, `struct mg_connection::send_mbuf`
and `struct mg_connection::recv_mbuf` respectively. When data arrives,
Mongoose appends received data to the `recv_mbuf` and triggers an `MG_EV_RECV`
event. The user may send data back by calling one of the output functions, like
`mg_send()` or `mg_printf()`. Output functions append data to the `send_mbuf`.
When Mongoose successfully writes data to the socket, it discards data from
`struct mg_connection::send_mbuf` and sends an `MG_EV_SEND` event. When the connection
is closed, an `MG_EV_CLOSE` event is sent.

每一个连接都由一个发送缓冲区和接收缓冲区.
当接收到数据时, MG将数据存入接收缓冲区并且触发一个`MG_EV_RECV`事件, 通知事件处理器进行处理.
用户调用数据发送函数(`mg_send()` or `mg_printf()`等)后, 
MG将数据存入发送缓冲区, 数据发送完毕后MG会自动清理掉缓冲区的数据,并触发一个`MG_EV_SEND`事件.




# 为什么不用epoll?

作者的回复:

I am not really convinced that using select() is the problem.
select() is slow when many file descriptors are checked for data. 
Usually listening sockets and all connected sockets are passed to select/epoll, and it could be many of those, if many clients are connected. 
Mongoose however is using select() for checking listening sockets only. 
Usually, there are few of them, and most of the time, only one (this is  set by -listening_ports parameter). 
If there are thousands listening sockets made, then yes, switching to epoll() would make sense, but I don't think anybody is doing this.
To make real performance boost on static content, Mongoose should use sendfile(2).












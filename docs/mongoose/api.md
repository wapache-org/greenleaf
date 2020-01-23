# mbuf

Mbufs are mutable/growing memory buffers, like C++ strings.
Mbuf can append data to the end of a buffer or insert data into arbitrary position in the middle of a buffer. 
The buffer grows automatically when needed.

```c
  struct mbuf {
    char *buf;   /* Buffer pointer */
    size_t len;  /* Data length. Data is located between offset 0 and len. */
    size_t size; /* Buffer size allocated by realloc(1). Must be >= len */
  };
```

size是缓冲区的容量, len是实际数据长度, buf是数据的第一个字节的指针

可以对缓冲区做以下操作

 * init: 初始化一个缓冲区
 * append: 追加数据
 * insert: 在指定位置插入数据
 * remove: 删除数据
 * clear: 清空缓冲区
 * free: Frees the space allocated for the mbuffer and resets the mbuf structure. 
 * append_and_free
 * resize
 * trim


```c
// 添加数据到缓冲区
size_t // 0: 内存溢出, 其他:实际添加的数据长度
mbuf_append(
    struct mbuf *,    // 添加到此缓冲区
    const void *data, // 待添加的数据
    size_t data_size  // 数据长度
);
```

# http

常用的几个结构体是:

 * http_message
 * mg_http_multipart_part
 * websocket_message

常用的几个操作有:
 * mg_connect_http/mg_connect_http_opt: 连接一个出站连接(譬如连接服务器)
 * mg_connect_ws

常用获取消息内容操作:

 * mg_get_http_header: 获取http头的值, 复杂的http头通常配合`mg_http_parse_header2`使用
 * mg_http_parse_header2: 解析http头的值
 * mg_get_http_var: 获取表单字段的值


认证常用操作:
 * mg_get_http_basic_auth: 获取basic认证头
 * mg_check_digest_auth: 
 * mg_http_create_digest_auth_header: 设置digest认证头

响应常用操作:
 * mg_http_send_error
 * mg_http_send_redirect

常用操作未写完, 待续...

```c
void mg_set_protocol_http_websocket(struct mg_connection *nc);
```

这个函数名其实应该是`mg_set_protocol_options_of_http_and_websocket`
它会添加一个内置的事件处理器到指定的连接,并且生成以下事件:

Attaches a built-in HTTP event handler to the given connection.
The user-defined event handler will receive following extra events:

- MG_EV_HTTP_REQUEST: HTTP request has arrived. Parsed HTTP request
 is passed as
    `struct http_message` through the handler's `void *ev_data` pointer.
- MG_EV_HTTP_REPLY: The HTTP reply has arrived. The parsed HTTP reply is
  passed as `struct http_message` through the handler's `void *ev_data`
  pointer.
- MG_EV_HTTP_CHUNK: The HTTP chunked-encoding chunk has arrived.
  The parsed HTTP reply is passed as `struct http_message` through the
  handler's `void *ev_data` pointer. `http_message::body` would contain
  incomplete, reassembled HTTP body.
  It will grow with every new chunk that arrives, and it can
  potentially consume a lot of memory. An event handler may process
  the body as chunks are coming, and signal Mongoose to delete processed
  body by setting `MG_F_DELETE_CHUNK` in `mg_connection::flags`. When
  the last zero chunk is received,
  Mongoose sends `MG_EV_HTTP_REPLY` event with
  full reassembled body (if handler did not signal to delete chunks) or
  with empty body (if handler did signal to delete chunks).
- MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: server has received the WebSocket
  handshake request. `ev_data` contains parsed HTTP request.
- MG_EV_WEBSOCKET_HANDSHAKE_DONE: server has completed the WebSocket
  handshake. `ev_data` is a `struct http_message` containing the
  client's request (server mode) or server's response (client).
  In client mode handler can examine `resp_code`, which should be 101.
- MG_EV_WEBSOCKET_FRAME: new WebSocket frame has arrived. `ev_data` is
  `struct websocket_message *`

When compiled with MG_ENABLE_HTTP_STREAMING_MULTIPART, Mongoose parses
multipart requests and splits them into separate events:

- MG_EV_HTTP_MULTIPART_REQUEST: Start of the request.
  This event is sent before body is parsed. After this, the user
  should expect a sequence of PART_BEGIN/DATA/END requests.
  This is also the last time when headers and other request fields are
  accessible.
- MG_EV_HTTP_PART_BEGIN: Start of a part of a multipart message.
  Argument: mg_http_multipart_part with var_name and file_name set
  (if present). No data is passed in this message.
- MG_EV_HTTP_PART_DATA: new portion of data from the multipart message.
  Argument: mg_http_multipart_part. var_name and file_name are preserved,
  data is available in mg_http_multipart_part.data.
- MG_EV_HTTP_PART_END: End of the current part. var_name, file_name are
  the same, no data in the message. If status is 0, then the part is
  properly terminated with a boundary, status < 0 means that connection
  was terminated.
- MG_EV_HTTP_MULTIPART_REQUEST_END: End of the multipart request.
  Argument: mg_http_multipart_part, var_name and file_name are NULL,
  status = 0 means request was properly closed, < 0 means connection
  was terminated (note: in this case both PART_END and REQUEST_END are
  delivered). 




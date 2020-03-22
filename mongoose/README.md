

mongoose是以一个头文件,一个源文件方式发布的, 单其实是多个文件的合并, 官方提供了分割和合并的脚本.

```

# 1. 分别打开mongoose.h和mongoose.c, 删掉里边的mongoose/src, 让分割后的源代码在当前目录.

# 2. 开始分割
./unamalgam.py mongoose.h
./unamalgam.py mongoose.c

# 3. 修改源码后, 可以重新合并成单个文件
./amalgam.py --prefix=MG $(cat mongoose.h.manifest) > mongoose.h
./amalgam.py --prefix=MG --public-header=mongoose.h $(cat mongoose.c.manifest) > mongoose.c

```

目前使用的版本是6.17, 后续如果要升级版本, 可以在分割前, 先做一次差异合并.

mongoose默认将cs_dbg.h头文件放在mongoose.c文件而不是mongoose.h, 因此无法打开调试, 如果要调试, 可以把这个文件移到mongoose.h




# Events

```c

/* Events. Meaning of event parameter (evp) is given in the comment. */
#define MG_EV_POLL 0    /* Sent to each connection on each mg_mgr_poll() call */
#define MG_EV_ACCEPT 1  /* New connection accepted. union socket_address * */
#define MG_EV_CONNECT 2 /* connect() succeeded or failed. int *  */
#define MG_EV_RECV 3    /* Data has been received. int *num_bytes */
#define MG_EV_SEND 4    /* Data has been written to a socket. int *num_bytes */
#define MG_EV_CLOSE 5   /* Connection is closed. NULL */
#define MG_EV_TIMER 6   /* now >= conn->ev_timer_time. double * */


/* HTTP and websocket events. void *ev_data is described in a comment. */
#define MG_EV_HTTP_REQUEST 100 /* struct http_message * */
#define MG_EV_HTTP_REPLY 101   /* struct http_message * */
#define MG_EV_HTTP_CHUNK 102   /* struct http_message * */
#define MG_EV_SSI_CALL 105     /* char * */
#define MG_EV_SSI_CALL_CTX 106 /* struct mg_ssi_call_ctx * */

#if MG_ENABLE_HTTP_WEBSOCKET
#define MG_EV_WEBSOCKET_HANDSHAKE_REQUEST 111 /* struct http_message * */
#define MG_EV_WEBSOCKET_HANDSHAKE_DONE 112    /* struct http_message * */
#define MG_EV_WEBSOCKET_FRAME 113             /* struct websocket_message * */
#define MG_EV_WEBSOCKET_CONTROL_FRAME 114     /* struct websocket_message * */
#endif

#if MG_ENABLE_HTTP_STREAMING_MULTIPART
#define MG_EV_HTTP_MULTIPART_REQUEST 121 /* struct http_message */
#define MG_EV_HTTP_PART_BEGIN 122        /* struct mg_http_multipart_part */
#define MG_EV_HTTP_PART_DATA 123         /* struct mg_http_multipart_part */
#define MG_EV_HTTP_PART_END 124          /* struct mg_http_multipart_part */
/* struct mg_http_multipart_part */
#define MG_EV_HTTP_MULTIPART_REQUEST_END 125



/* MQTT event types */
#define MG_MQTT_EVENT_BASE 200
#define MG_EV_MQTT_CONNECT (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_CONNECT)
#define MG_EV_MQTT_CONNACK (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_CONNACK)
#define MG_EV_MQTT_PUBLISH (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_PUBLISH)
#define MG_EV_MQTT_PUBACK (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_PUBACK)
#define MG_EV_MQTT_PUBREC (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_PUBREC)
#define MG_EV_MQTT_PUBREL (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_PUBREL)
#define MG_EV_MQTT_PUBCOMP (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_PUBCOMP)
#define MG_EV_MQTT_SUBSCRIBE (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_SUBSCRIBE)
#define MG_EV_MQTT_SUBACK (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_SUBACK)
#define MG_EV_MQTT_UNSUBSCRIBE (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_UNSUBSCRIBE)
#define MG_EV_MQTT_UNSUBACK (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_UNSUBACK)
#define MG_EV_MQTT_PINGREQ (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_PINGREQ)
#define MG_EV_MQTT_PINGRESP (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_PINGRESP)
#define MG_EV_MQTT_DISCONNECT (MG_MQTT_EVENT_BASE + MG_MQTT_CMD_DISCONNECT)


#define MG_COAP_EVENT_BASE 300
#define MG_EV_COAP_CON (MG_COAP_EVENT_BASE + MG_COAP_MSG_CON)
#define MG_EV_COAP_NOC (MG_COAP_EVENT_BASE + MG_COAP_MSG_NOC)
#define MG_EV_COAP_ACK (MG_COAP_EVENT_BASE + MG_COAP_MSG_ACK)
#define MG_EV_COAP_RST (MG_COAP_EVENT_BASE + MG_COAP_MSG_RST)






```

MG_EV_POLL: Sent to all connections on each invocation of mg_mgr_poll(). This event could be used to do any housekeeping, for example check whether a certain timeout has expired and closes the connection or send heartbeat message, etc.

MG_EV_TIMER: Sent to the connection if mg_set_timer() was called.
MG_EV_POLL: Sent to all connections on each invocation of mg_mgr_poll(). This event could be used to do any housekeeping, for example check whether a certain timeout has expired and closes the connection or send heartbeat message, etc.

MG_EV_TIMER: Sent to the connection if mg_set_timer() was called.

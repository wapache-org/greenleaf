# 预处理选项

# Enabling flags

# - MG_ENABLE_SSL Enable SSL/TLS support (OpenSSL API)
# - MG_ENABLE_IPV6 Enable IPv6 support
# - MG_ENABLE_MQTT enable MQTT client (on by default, set to 0 to disable)
# - MG_ENABLE_MQTT_BROKER enable MQTT broker
# - MG_ENABLE_DNS_SERVER enable DNS server
# - MG_ENABLE_COAP enable CoAP protocol
# - MG_ENABLE_HTTP Enable HTTP protocol support (on by default, set to 0 to disable)
# - MG_ENABLE_HTTP_CGI Enable CGI support
# - MG_ENABLE_HTTP_SSI Enable Server Side Includes support
# - MG_ENABLE_HTTP_SSI_EXEC Enable SSI exec operator
# - MG_ENABLE_HTTP_WEBDAV enable WebDAV extensions to HTTP
# - MG_ENABLE_HTTP_WEBSOCKET enable WebSocket extension to HTTP (on by default, =0 to disable)
# - MG_ENABLE_BROADCAST enable mg_broadcast() API
# - MG_ENABLE_GETADDRINFO enable getaddrinfo() in mg_resolve2()
# - MG_ENABLE_THREADS enable mg_start_thread() API

# Disabling flags

# - MG_DISABLE_HTTP_DIGEST_AUTH disable HTTP Digest (MD5) authorisation support
# - CS_DISABLE_SHA1 disable SHA1 support (used by WebSocket)
# - CS_DISABLE_MD5 disable MD5 support (used by HTTP auth)
# - MG_DISABLE_HTTP_KEEP_ALIVE useful for embedded systems to save resources

# Platform specific

# Mongoose tries to detect the target platform whenever possible, but in some cases you have
# to explicitly declare some peculiarities of your target, such as:

# - MG_CC3200: enable workarounds for the TI CC3200 target.
# - MG_ESP8266: enable workarounds for the ESP8266 target, add RTOS_SDK to specify the RTOS SDK flavour.

# Tunables

# - MG_MALLOC, MG_CALLOC, MG_REALLOC, MG_FREE allow you to a use custom
#   memory allocator, e.g. -DMG_MALLOC=my_malloc
# - MG_USE_READ_WRITE when set replaces calls to recv with read and send with write,
#   thus enabling to add any kind of file descriptor (files, serial devices) to an event manager.
# - MG_SSL_CRYPTO_MODERN, MG_SSL_CRYPTO_OLD - choose either "Modern" or "Old" ciphers
#   instead of the default "Intermediate" setting.
#   See this article for details.
# - MG_USER_FILE_FUNCTIONS allow you to use custom file operation, by defining you own mg_stat, mg_fopen, mg_open, mg_fread and mg_fwrite functions

# 设置预处理选项
# add_definitions(-DMG_DISABLE_MQTT -DMG_DISABLE_COAP)

add_definitions(-DMG_ENABLE_DNS_SERVER)


# 设置目标编译选项
# add_compile_definitions(XXX )
# target_compile_definitions(mongoose XXX)
# set(CMAKE_C_FLAGS "-std=c11 ${CMAKE_CXX_FLAGS}")


aux_source_directory(. DIR_MONGOOSE_SRCS)
add_library(mongoose ${DIR_MONGOOSE_SRCS})
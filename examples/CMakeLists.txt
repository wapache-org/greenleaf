# 设置预处理选项
# add_definitions(-DMG_DISABLE_MQTT -DMG_DISABLE_COAP)

# 设置目标编译选项
# add_compile_definitions(XXX )
# target_compile_definitions(mongoose XXX)
# set(CMAKE_C_FLAGS "-std=c11 ${CMAKE_CXX_FLAGS}")

# 设置编译源代码目录
# aux_source_directory(. DIR_EXAMPLE_SRCS)

# 设置编译目标
# add_library(mongoose ${DIR_EXAMPLE_SRCS})

# add_executable(api_server api_server.c db_plugin_sqlite.c)
# target_link_libraries(api_server mongoose sqlite lua mustache json-c m dl readline pthread)

add_executable(web_server_example web_server_example.c)
target_link_libraries(web_server_example mongoose)

add_executable(cgi_example cgi_example.c)
target_link_libraries(cgi_example mongoose m dl readline pthread)

add_executable(websocket_example websocket_example.c)
target_link_libraries(websocket_example mongoose)

add_executable(websocket_client_example websocket_client_example.c)
target_link_libraries(websocket_client_example mongoose)

add_executable(sqlite_example sqlite_example.c)
target_link_libraries(sqlite_example mongoose sqlite dl pthread)

add_executable(api_server_example api_server_example.c db_plugin_sqlite.c)
target_link_libraries(api_server_example mongoose sqlite dl pthread)

add_executable(auth_cookie_example auth_cookie_example.c)
target_link_libraries(auth_cookie_example mongoose)

add_definitions(-DCS_ENABLE_UTF8)
add_executable(v7js_example v7js_example.c)
target_link_libraries(v7js_example v7js m mongoose)

link_directories("/usr/local/lib/quickjs" "/opt/pgsql-12/lib")
add_executable(qjs_example qjs_example.c)
target_link_libraries(qjs_example quickjs m dl)

add_executable(postgres_example postgres_example.c)
target_link_libraries(postgres_example m dl pq)

add_executable(qjs_postgres_example qjs_postgres_example.c)
target_link_libraries(qjs_postgres_example quickjs pq m dl)
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

add_executable(qjs_example qjs_example.c)
target_link_libraries(qjs_example quickjs m dl)

add_executable(postgres_example postgres_example.c)
target_link_libraries(postgres_example m dl pq)

add_executable(postgres_static_example postgres_example.c)
target_link_libraries(postgres_static_example pq.a pgport.a pgcommon.a pthread)

add_executable(qjs_postgres_example qjs_postgres_example.c)
target_link_libraries(qjs_postgres_example quickjs pq m dl)

add_executable(qjs_mongoose_example qjs_mongoose_example.c)
target_link_libraries(qjs_mongoose_example mongoose quickjs pq m dl)
add_custom_command(
    TARGET qjs_mongoose_example
    POST_BUILD 
    COMMAND echo "copy qjs_mongoose_template_engine.js"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/qjs_mongoose_template_engine.js ${CMAKE_CURRENT_BINARY_DIR}/qjs_mongoose_template_engine.js
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/qjs_mongoose_template_engine.js
)
add_custom_command(
    TARGET qjs_mongoose_example
    POST_BUILD 
    COMMAND echo "copy qjs_mongoose_handler.js"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/qjs_mongoose_handler.js ${CMAKE_CURRENT_BINARY_DIR}/qjs_mongoose_handler.js
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/qjs_mongoose_handler.js
)

add_executable(qjs_module_example qjs_module_example.c)
target_link_libraries(qjs_module_example quickjs m dl)


add_executable(cronexpr_example cronexpr_example.c)
target_link_libraries(cronexpr_example cron)

add_executable(crontab_example crontab_example.c ../common/logger.c)
target_link_libraries(crontab_example cron)

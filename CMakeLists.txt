cmake_minimum_required(VERSION 3.0.0)
project(mongoose_lua LANGUAGES C VERSION 0.1.0)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_definitions(-DMG_ENABLE_DNS_SERVER)

add_compile_options(-Wl,-E)

# add the source in project root directory
aux_source_directory(. DIR_SRCS)

# add header file include directories
include_directories(./ ./lua ./mongoose ./mustache ./sqlite)

# add block directories
add_subdirectory(lua)
add_subdirectory(mongoose)
add_subdirectory(mustache)
add_subdirectory(sqlite)
add_subdirectory(examples)

# add target
add_executable(mongoose_lua main.c)
target_link_libraries(mongoose_lua mongoose lua mustache json-c m dl readline)

# add_executable(api_server api_server.c db_plugin_sqlite.c)
# target_link_libraries(api_server mongoose sqlite lua mustache json-c m dl readline pthread)


# add_executable(restful_server restful_server.c)
# target_link_libraries(restful_server mongoose pthread)

# add_executable(restful_client restful_client.c)
# target_link_libraries(restful_client mongoose pthread)



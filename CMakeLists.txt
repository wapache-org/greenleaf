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
include_directories(./ ./mongoose ./lua ./v7js ./mustache ./sqlite "/opt/pgsql-12/include")

# add lib directories
link_directories("/usr/local/lib/quickjs" "/opt/pgsql-12/lib")

# add block directories
add_subdirectory(lua)
add_subdirectory(v7js)
add_subdirectory(mongoose)
add_subdirectory(mustache)
add_subdirectory(sqlite)
add_subdirectory(examples)

# add target
add_executable(mongoose_lua main.c)
add_dependencies(mongoose_lua copy_files)
target_link_libraries(mongoose_lua mongoose lua quickjs pq sqlite json-c m dl readline)

# add file copy target
FILE(GLOB QUICKJS_MODULE_FILES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/quickjs_modules/*.js
)
FILE(GLOB LUA_MODULE_FILES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/lua_modules/*.lua
)
add_custom_target(copy_files ALL 
  VERBATIM 
  COMMAND_EXPAND_LISTS 
  COMMAND ${CMAKE_COMMAND} -E make_directory 
    "${CMAKE_CURRENT_BINARY_DIR}/quickjs_modules" 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different  
    "${QUICKJS_MODULE_FILES}" 
    "${LUA_MODULE_FILES}"
    "${CMAKE_CURRENT_BINARY_DIR}/quickjs_modules/" 
)
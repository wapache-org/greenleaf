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
include_directories(
  ./ 
  ./mongoose 
  ./lua 
  ./mustache 
  ./json-c
  ./v7js
  ./sqlite
  ./lib/include
)

# add lib directories
link_directories(
  "/usr/local/lib/quickjs" 
  lib/libpq
)

# add block directories
add_subdirectory(lua)
add_subdirectory(v7js)
add_subdirectory(mongoose)
add_subdirectory(mustache)
add_subdirectory(sqlite)
add_subdirectory(examples)
add_subdirectory(json-c)

# add grobal libs
# link_libraries(mongoose lua quickjs pq sqlite m dl readline)

# add target

add_executable(mongoose_lua main.c)
# add_dependencies(mongoose_lua copy_files)
target_link_libraries(mongoose_lua mongoose lua quickjs sqlite m dl
  pq
  ${CMAKE_CURRENT_BINARY_DIR}/json-c/libjson-c.a
)

# add file copy target
FILE(GLOB QUICKJS_MODULE_FILES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/quickjs_modules/*.js
)
FILE(GLOB LUA_MODULE_FILES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/lua_modules/*.lua
)

SET(GPMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/gpmon)
add_custom_target(
  build_gpmon ALL 
  DEPENDS mongoose_lua
  VERBATIM 
  COMMAND_EXPAND_LISTS

  COMMAND ${CMAKE_COMMAND} -E remove_directory
    ${GPMON_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory
    ${GPMON_DIR}

  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/build/mongoose_lua
    ${GPMON_DIR}/gpmon

  #COMMAND ${CMAKE_COMMAND} -E make_directory ${GPMON_DIR}/templates
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/templates
    ${GPMON_DIR}/templates

  #COMMAND ${CMAKE_COMMAND} -E make_directory ${GPMON_DIR}/qjs_modules
  COMMAND ${CMAKE_COMMAND} -E copy_directory  
    ${CMAKE_CURRENT_SOURCE_DIR}/quickjs_modules
    ${GPMON_DIR}/qjs_modules

  #COMMAND ${CMAKE_COMMAND} -E make_directory ${GPMON_DIR}/lib
  COMMAND ${CMAKE_COMMAND} -E copy  
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libpq/libpq.so.5
    ${GPMON_DIR}/lib/libpq.so.5
)

# install



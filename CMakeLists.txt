cmake_minimum_required(VERSION 3.0.0)
project(greenleaf LANGUAGES C VERSION 0.1.0)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_definitions(-DMG_ENABLE_DNS_SERVER -DMG_ENABLE_THREADS -DMG_ENABLE_HTTP_STREAMING_MULTIPART)

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
  ./sqlite
  ./cron
  ./libraries/libpq/include
  ./libraries/quickjs/include
  ./libraries/libssh/include
)

# add lib directories
link_directories(
  libraries/quickjs/lib/quickjs
  libraries/libpq/lib
  libraries/libssh/lib
  libraries/libyaml/lib
)

# add block directories
add_subdirectory(common)
add_subdirectory(json-c)
add_subdirectory(mongoose)
add_subdirectory(cron)
add_subdirectory(ssh)
add_subdirectory(sqlite)
add_subdirectory(lua)
add_subdirectory(mustache)
add_subdirectory(examples)

# add grobal libs
# link_libraries(mongoose lua quickjs pq sqlite m dl readline)

# add target
add_executable(greenleaf main.c)
# add_dependencies(greenleaf copy_files)
target_link_libraries(greenleaf 
  common
  mongoose 
  json-c yaml
  crontab
  lua quickjs.a
  sqlite
  pq.a pgport.a pgcommon.a 
  m dl pthread
)

# libssh 依赖的glibc的版本太高, 在centos上运行不了
if(false)
add_definitions(-DGL_ENABLE_WEBSSH)
target_link_libraries(greenleaf 
  ssh
  crypto
  ssh_common
)
endif()

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
  DEPENDS greenleaf
  VERBATIM 
  COMMAND_EXPAND_LISTS

  COMMAND ${CMAKE_COMMAND} -E remove_directory
    ${GPMON_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory
    ${GPMON_DIR}

  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/build/greenleaf
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
  # COMMAND ${CMAKE_COMMAND} -E copy  
  #   ${CMAKE_CURRENT_SOURCE_DIR}/lib/libpq/libpq.so.5
  #   ${GPMON_DIR}/lib/libpq.so.5
)

# install



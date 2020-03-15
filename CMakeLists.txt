cmake_minimum_required(VERSION 3.0.0)
project(greenleaf LANGUAGES C VERSION 0.1.0)

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
)

# add block directories
add_subdirectory(lua)
add_subdirectory(mongoose)
add_subdirectory(mustache)
add_subdirectory(sqlite)
add_subdirectory(examples)
add_subdirectory(json-c)
add_subdirectory(ssh)
add_subdirectory(cron)

# add grobal libs
# link_libraries(mongoose lua quickjs pq sqlite m dl readline)

# add target
set(ssh_SRCS1
  ssh/authentication.c
  ssh/knownhosts.c
  ssh/connect_ssh.c
)

add_executable(greenleaf main.c ${ssh_SRCS1})
# add_dependencies(greenleaf copy_files)
target_link_libraries(greenleaf 
  mongoose json-c
  lua quickjs.a
  sqlite
  pq.a pgport.a pgcommon.a 
  m dl pthread
  ssh
  cron
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



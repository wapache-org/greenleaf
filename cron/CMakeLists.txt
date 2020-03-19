# 设置预处理选项
# add_definitions(-DMG_DISABLE_MQTT -DMG_DISABLE_COAP)

# 设置目标编译选项
# add_compile_definitions(XXX )
# target_compile_definitions(mongoose XXX)
# set(CMAKE_C_FLAGS "-std=c11 ${CMAKE_CXX_FLAGS}")

aux_source_directory(. DIR_CRON_SRCS)
add_library(crontab ${DIR_CRON_SRCS} ../common/logger.c)
target_link_libraries(crontab 
  json-c
  m dl pthread
)
/**
 * 长期来看, 还是引入log4c比较好
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>

#define LOG_VERSION "0.1.0"

typedef void (*logger_lock_func)(void *udata, int lock);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL, LOG_NONE };

#define logger_trace(...) logger_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define logger_debug(...) logger_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define logger_info(...)  logger_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define logger_warn(...)  logger_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define logger_error(...) logger_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define logger_fatal(...) logger_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void logger_set_udata(void *udata);
void logger_set_lock(logger_lock_func fn);
void logger_set_fp(FILE *fp);
void logger_set_level(int level);
void logger_set_level_by_name(const char* level);
void logger_set_quiet(int enable);

void logger_log(int level, const char *file, int line, const char *fmt, ...);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "logger.h"

#ifndef LOG_USE_COLOR
#define LOG_USE_COLOR 1
#endif

static struct {
  void *udata;
  logger_lock_func lock;
  FILE *fp;  // 文件描述符
  int level; // 日志级别 
  int quiet; // 只输出到文件
} L;


static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif


static void lock(void)   {
  if (L.lock) {
    L.lock(L.udata, 1);
  }
}


static void unlock(void) {
  if (L.lock) {
    L.lock(L.udata, 0);
  }
}


void logger_set_udata(void *udata) {
  L.udata = udata;
}


void logger_set_lock(logger_lock_func fn) {
  L.lock = fn;
}


void logger_set_fp(FILE *fp) {
  L.fp = fp;
}


void logger_set_level(int level) {
  L.level = level;
}

#define STR_IS_EQUALS(a,b) memcmp(a,b, strlen(b))==0
void logger_set_level_by_name(const char* level)
{
    if(STR_IS_EQUALS( level, "trace")){
      logger_set_level(LOG_TRACE);
    }else if(STR_IS_EQUALS( level, "debug")){
      logger_set_level(LOG_DEBUG);
    }else if(STR_IS_EQUALS( level, "info")){
      logger_set_level(LOG_INFO);
    }else if(STR_IS_EQUALS( level, "warn")){
      logger_set_level(LOG_WARN);
    }else if(STR_IS_EQUALS( level, "error")){
      logger_set_level(LOG_ERROR);
    }else if(STR_IS_EQUALS( level, "fatal")){
      logger_set_level(LOG_FATAL);
    }else if(STR_IS_EQUALS( level, "none")){
      logger_set_level(LOG_NONE);
    }
}

void logger_set_quiet(int enable) {
  L.quiet = enable ? 1 : 0;
}


void logger_log(int level, const char *file, int line, const char *fmt, ...) {
  if (level < L.level) {
    return;
  }

  /* Acquire lock */
  lock();

  /* Get current time */
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  /* Log to stdout/stderr */
  if (!L.quiet) {
    FILE* fd = level>3 ? stderr : stdout;
    va_list args;
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(
      fd, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
      buf, level_colors[level], level_names[level], file, line);
#else
    fprintf(fd, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
#endif
    va_start(args, fmt);
    vfprintf(fd, fmt, args);
    va_end(args);
    fprintf(fd, "\n");
    fflush(fd);
  }

  /* Log to file */
  if (L.fp) {
    va_list args;
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
    fprintf(L.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
    va_start(args, fmt);
    vfprintf(L.fp, fmt, args);
    va_end(args);
    fprintf(L.fp, "\n");
    fflush(L.fp);
  }

  /* Release lock */
  unlock();
}
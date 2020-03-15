#ifndef CRONEXPR_H
#define CRONEXPR_H

#if defined(__cplusplus) && !defined(CRON_COMPILE_AS_CXX)
extern "C" {
#endif

#ifndef ANDROID
#include <time.h>
#else /* ANDROID */
#include <time64.h>
#endif /* ANDROID */

// Linux crontab :
// # Example of job definition:
// # .---------------- minute (0 - 59)
// # |  .------------- hour (0 - 23)
// # |  |  .---------- day of month (1 - 31)
// # |  |  |  .------- month (1 - 12) OR jan,feb,mar,apr ...
// # |  |  |  |  .---- day of week (0 - 6) (Sunday=0 or 7) OR sun,mon,tue,wed,thu,fri,sat
// # |  |  |  |  |
// # *  *  *  *  * user-name  command to be executed

/**
 * cron表达式结构体
 */
typedef struct {
  /** 秒 */
  char *seconds;
  /** 分 */
  char *minutes;
  /** 时 */
  char *hours;
  /** 星期 */
  char *days_of_week;
  /** 日 */
  char *days_of_month;
  /** 月 */
  char *months;
} cronexpr;

/**
 * Parses specified cron expression.
 *
 * @param expression cron expression as nul-terminated string,
 *        should be no longer that 256 bytes
 * @param error output error message, will be set to string literal
 *        error message in case of error. Will be set to NULL on success.
 *        The error message should NOT be freed by client.
 * @return parsed cron expression in case of success. Returned expression
 *        must be freed by client using 'cronexpr_free' function.
 *        NULL is returned on error.
 */
cronexpr* cronexpr_parse(const char *expression, const char **error);

/**
 * Frees the memory allocated by the specified cron expression
 *
 * @param expr parsed cron expression to free
 */
void cronexpr_free(cronexpr *expr);

/**
 * Uses the specified expression to calculate the next 'fire' date after
 * the specified date. All dates are processed as UTC (GMT) dates
 * without timezones information. To use local dates (current system timezone)
 * instead of GMT compile with '-DCRON_USE_LOCAL_TIME'
 *
 * @param expr parsed cron expression to use in next date calculation
 * @param date start date to start calculation from
 * @return next 'fire' date in case of success, '((time_t) -1)' in case of
 * error.
 */
time_t cronexpr_next(cronexpr *expr, time_t date);

#if defined(__cplusplus) && !defined(CRON_COMPILE_AS_CXX)
}  // extern "C"
#endif

#endif /* CRONEXPR_H */
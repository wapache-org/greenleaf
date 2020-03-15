#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "cronexpr.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define TEST_BASE_TIME "2017-08-29 22:00:20"
double cs_timegm(const struct tm *tm);

struct test_entry
{
    char* expr;
    char* expected[3];
};

/* TEST_BASE_TIME is 2017-08-29 22:00:20 UTC+-X */
static struct test_entry s_te[] = {/* Local time */
    {.expr = "* * * * * *",
    .expected[0] = "2017-08-29 22:00:21",
    .expected[1] = "2017-08-29 22:00:22",
    .expected[2] = "2017-08-29 22:00:23"},
    {.expr = "0 * * * * *",
    .expected[0] = "2017-08-29 22:01:00",
    .expected[1] = "2017-08-29 22:02:00",
    .expected[2] = "2017-08-29 22:03:00"},
    {.expr = "0 0 * * * *",
    .expected[0] = "2017-08-29 23:00:00",
    .expected[1] = "2017-08-30 00:00:00",
    .expected[2] = "2017-08-30 01:00:00"},
    {.expr = "0 0 0 * * *",
    .expected[0] = "2017-08-30 00:00:00",
    .expected[1] = "2017-08-31 00:00:00",
    .expected[2] = "2017-09-01 00:00:00"},
    {.expr = "0 0 0 1 * *",
    .expected[0] = "2017-09-01 00:00:00",
    .expected[1] = "2017-10-01 00:00:00",
    .expected[2] = "2017-11-01 00:00:00"},
    {.expr = "10 * * * * *",
    .expected[0] = "2017-08-29 22:01:10",
    .expected[1] = "2017-08-29 22:02:10",
    .expected[2] = "2017-08-29 22:03:10"},
    {.expr = "10 */1 * * * *",
    .expected[0] = "2017-08-29 22:01:10",
    .expected[1] = "2017-08-29 22:02:10",
    .expected[2] = "2017-08-29 22:03:10"},
    {.expr = "*/15 * * * * *",
    .expected[0] = "2017-08-29 22:00:30",
    .expected[1] = "2017-08-29 22:00:45",
    .expected[2] = "2017-08-29 22:01:00"},
    {.expr = "* * * 5 9 *",
    .expected[0] = "2017-09-05 00:00:00",
    .expected[1] = "2017-09-05 00:00:01",
    .expected[2] = "2017-09-05 00:00:02"},
    {.expr = "0 0 0 31 * *",
    .expected[0] = "2017-08-31 00:00:00",
    .expected[1] = "2017-10-31 00:00:00",
    .expected[2] = "2017-12-31 00:00:00"},
    {.expr = "0 0 0 * * 3",
    .expected[0] = "2017-08-30 00:00:00",
    .expected[1] = "2017-09-06 00:00:00",
    .expected[2] = "2017-09-13 00:00:00"},
    {.expr = "0 * * * * 3",
    .expected[0] = "2017-08-30 00:00:00",
    .expected[1] = "2017-08-30 00:01:00",
    .expected[2] = "2017-08-30 00:02:00"},
    {.expr = "0 0 9 1-7 * 1",
    .expected[0] = "2017-09-04 09:00:00",
    .expected[1] = "2017-10-02 09:00:00",
    .expected[2] = "2017-11-06 09:00:00"}
};
struct sorted_test_entry {
  int id;
  time_t expected;
};

extern long timezone;

static char s_tz[8] = {0};
static int s_num_tests = 0;
static char s_str_time[32] = {0};
static struct sorted_test_entry *s_sorted_te = NULL;
static int s_current_id = 0;

static void s_get_timezone(void) {
  tzset();
  int t = timezone / 60 / 60;
  if (t == 0) {
    strcpy(s_tz, "UTC");
  } else {
    bool neg = false;
    if (t < 0) {
      neg = true;
      t *= -1;
    }
    sprintf(s_tz, "UTC%c%d", neg ? '+' : '-', t);
  }
}

static struct tm s_tm = {0};
static struct tm *s_cron_test_str2tm(const char *date) {
  memset(&s_tm, 0, sizeof s_tm);

  sscanf(date, "%04d-%02d-%02d %02d:%02d:%02d", 
    &s_tm.tm_year, &s_tm.tm_mon, &s_tm.tm_mday, 
    &s_tm.tm_hour, &s_tm.tm_min, &s_tm.tm_sec
  );

  s_tm.tm_mon -= 1;
  s_tm.tm_year -= 1900;
  s_tm.tm_isdst = -1;

  return &s_tm;
}

static time_t s_cron_test_str2timeloc(const char *date) {
  return mktime(s_cron_test_str2tm(date));
}

static time_t s_cron_test_str2timegm(const char *date) {
  return (time_t) cs_timegm(s_cron_test_str2tm(date));
}

static char *s_cron_test_tm2str(struct tm *t, const char *gmt) {
  size_t sz = ARRAY_SIZE(s_str_time);
  memset(s_str_time, 0, sz);
  snprintf(s_str_time, sz - 1, "%04d-%02d-%02d %02d:%02d:%02d %s",
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min,
           t->tm_sec, gmt);
  return s_str_time;
}

static char *s_cron_test_timeloc2str(time_t date) {
  struct tm t;
  localtime_r(&date, &t);
  return s_cron_test_tm2str(&t, s_tz);
}

static char *s_cron_test_timegm2str(time_t date) {
  struct tm t;
  gmtime_r(&date, &t);
  return s_cron_test_tm2str(&t, "UTC");
}

double cs_timegm(const struct tm *tm) {
  /* Month-to-day offset for non-leap-years. */
  static const int month_day[12] = {0,   31,  59,  90,  120, 151,
                                    181, 212, 243, 273, 304, 334};

  /* Most of the calculation is easy; leap years are the main difficulty. */
  int month = tm->tm_mon % 12;
  int year = tm->tm_year + tm->tm_mon / 12;
  int year_for_leap;
  signed long rt;

  if (month < 0) { /* Negative values % 12 are still negative. */
    month += 12;
    --year;
  }

  /* This is the number of Februaries since 1900. */
  year_for_leap = (month > 1) ? year + 1 : year;

  rt =
      tm->tm_sec /* Seconds */
      +
      60 *
          (tm->tm_min /* Minute = 60 seconds */
           +
           60 * (tm->tm_hour /* Hour = 60 minutes */
                 +
                 24 * (month_day[month] + tm->tm_mday - 1 /* Day = 24 hours */
                       + 365 * (year - 70)                /* Year = 365 days */
                       + (year_for_leap - 69) / 4 /* Every 4 years is leap... */
                       - (year_for_leap - 1) / 100 /* Except centuries... */
                       + (year_for_leap + 299) / 400))); /* Except 400s. */
  return rt < 0 ? -1 : (double) rt;
}

int main(int argc, char* argv[])
{
    // 
    const char *err = NULL;

    for (size_t index = 0; index < ARRAY_SIZE(s_te); index++)
    {
        
        struct test_entry entry = s_te[index];

        cronexpr* expr = cronexpr_parse(entry.expr, &err);
        if(expr==NULL){
            printf("Error: %s\n", err);
            exit(1);
        }

        time_t base = s_cron_test_str2timegm(TEST_BASE_TIME);

        printf("%02d: expr = %s, base_time = %s :\n", index, entry.expr, TEST_BASE_TIME);

        for (size_t i = 0; i < 3; i++)
        {
            time_t next = cronexpr_next(expr, base);
            printf("\tactual = %s, expected = %s UTC\n", s_cron_test_timegm2str(next), entry.expected[i]);
            base = next;
        }
        // FIXED 有一部分触发时间不对!!!

        //printf("%s %s %s %s %s %s\n", expr->seconds, expr->minutes, expr->hours, expr->months, expr->days_of_week, expr->days_of_month);

        cronexpr_free(expr);

    }
    return 0;
}


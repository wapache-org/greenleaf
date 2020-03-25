/*
 * 存储格式:
 *
 * ```json
 * {"items":[
 *   ["1", {
 *     "at": "0 0 7 * * MON-FRI",
 *     "enable": true,
 *     "action": "foo",
 *     "payload": {"a": 1, "b": 2}
 *   }],
 *   ["2", {
 *     "at": "0 30 23 30 * *",
 *     "enable": true,
 *     "action": "bar"
 *   }]
 * ]}
 * ```
 *
 * This file is maintained by a set of API functions (see below).
 *
 * Obviously, crontab file contains a set of cron jobs, each of which consists,
 * at least, of the cron expression like `0 0 7 * * MON-FRI` and an action to be taken. 
 * 
 * 一个crontab文件包含若干cron jobs, 每个job由一个cron表达式和一个action组成, action由命令和参数组成.
 * 
 * Action is just a string, in the
 * example above there are two actions: `foo` and `bar`. 
 * Additionally, there
 * can be a `payload`, which is an arbitrary JSON. Payload is just a set of
 * parameters for the action.
 *
 * Obviously, there should be a mapping between those string actions and the
 * corresponding functions to be called; this is what
 * `crontab_register_handler()` is for.
 * 命令和函数的映射关系由`crontab_register_handler()`这个函数指定
 *
 * Example:
 *
 * ```c
 * static void my_foo_cb(char* action, char* payload, void *userdata) {
 *   LOG(LL_INFO, ("Crontab foo job fired! Payload: %.*s", payload.len, payload.p));
 *   (void) action;
 *   (void) userdata;
 * }
 *
 * // Somewhere else:
 * crontab_register_handler("foo", my_foo_cb, NULL);
 * ```
 *
 * The code above maps action `foo` in the JSON to the callback `my_foo_cb`.
 */

#ifndef CRONTAB_H_
#define CRONTAB_H_

#include "cronexpr.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Invalid value for the crontab job id.
 */
#define CRONTAB_INVALID_JOB_ID ((crontab_job_id_t) 0)

struct crontab_job;
typedef struct crontab_job crontab_job;

/*
 * Calculate the next fire date after the specified date, using crontab ID
 * (returned by all cron RPC methods)
 */
typedef time_t (*crontab_job_timer)(
    cronexpr *expr, 
    time_t date
);
typedef int (*crontab_job_trigger)(
    crontab_job* job,
    void *user_data
);
typedef int (*crontab_job_runner)(
    const int id,
    const char* name,
    const char* action, 
    const char* payload,
    void *userdata
);
int crontab_job_trigger_default(
    crontab_job* job,
    void *user_data
);
int crontab_job_runner_default(
    const int id,
    const char* name,
    const char* action, 
    const char* payload,
    void *userdata
);
struct crontab_job {
    int id;
    char* name;

    cronexpr* cron_expr;
    char* action;
    char* payload;

    int enable; // 1: true, 0: false
    /** 装载后是否立刻执行一次, 1: true, 0: false */
    int trigger_on_load;

    time_t last_trigger_time;
    time_t next_trigger_time;

    crontab_job_timer timer;
    crontab_job_trigger trigger;
    crontab_job_runner runner;
};

typedef struct {
    char* at;
    char* action;
    char* payload;
} crontab_job_log;

/**
 * crontab job结构体
 */
typedef struct {
  /** 文件路径 */
  char *path; // 注意, 释放crontab时,不会free这个字段, 需要调用者自己控制, 因为有可能这个值不是指向堆内存
  long next_id;
  /** 时 */
  int job_size;
  /** 分 */
  crontab_job** jobs;
} crontab;


int crontab_new(crontab** crontab);
int crontab_free(crontab* crontab);

/*
 * Reads crontab from the given file, and adds the jobs to cron library.
 * If NULL is given as json_path, default crontab.json path will be used.
 */
int crontab_load(crontab* crontab);

/*
 * Writes crontab to the given file, and adds the jobs to cron library.
 * If NULL is given as json_path, default crontab.json path will be used.
 */
int crontab_save(crontab* crontab);

int crontab_new_job(crontab_job** job);

int crontab_free_job(crontab_job* job);

/*
 * Add a new job. Passed string data is not retained. If `pid` is not NULL,
 * resulting job id is written there.
 *
 * Returns true in case of success, false otherwise.
 *
 * If `perr` is not NULL, the error message will be written there (or NULL
 * in case of success). The caller should free the error message.
 */
int crontab_add_job(
    crontab* crontab,
    crontab_job* job
);

/*
 * Remove a job by its id.
 *
 * Returns true in case of success, false otherwise.
 *
 * If `perr` is not NULL, the error message will be written there (or NULL
 * in case of success). The caller should free the error message.
 */
int crontab_remove_job(crontab* crontab, crontab_job* job);

/*
 * Deactivate all existing cron jobs
 */
int crontab_clear_jobs(crontab* crontab);


// /*
//  * Edit a job by its id. 
//  * 
//  * Passed string data is not retained.
//  *
//  * Returns true in case of success, false otherwise.
//  *
//  * If `perr` is not NULL, the error message will be written there (or NULL
//  * in case of success). The caller should free the error message.
//  */
// bool crontab_job_edit(
//     crontab_job_id_t id, char* at,
//     bool enable, char* action,
//     char* payload, char **perr
// );

/*
 * Get job details by the job id. 
 * 
 * All output pointers (`at`, `enable`, `action`, `payload`) are optional (allowed to be NULL). 
 * For non-NULL string outputs (`at`, `action` and `payload`), 
 * the memory is allocated separately and the caller should free it.
 *
 * Returns true in case of success, false otherwise.
 *
 * If `perr` is not NULL, the error message will be written there (or NULL in case of success). 
 * The caller should free the error message.
 */
int crontab_get_job(
    crontab_job* job, 
    char **perr
);

/*
 * Callback for `crontab_iterate()`; 
 * all string data is invalidated when the callback returns.
 */
typedef int (*crontab_iterate_cb)(
    crontab_job* job,
    void *user_data
);
/*
 * Iterate over all jobs in crontab, see `crontab_iterate_cb` for details.
 *
 * Returns true in case of success, false otherwise.
 *
 * If `perr` is not NULL, the error message will be written there (or NULL
 * in case of success). The caller should free the error message.
 */
int crontab_iterate(
    crontab* crontab,
    crontab_iterate_cb cb, 
    void *userdata
);



/*
 * Prototype for a job handler to be registered with `crontab_register_handler()`.
 */
typedef void (*crontab_cb)(
    char* action, 
    char* payload,
    void *userdata
);
/*
 * Add a handler for the given string action
 *
 * Example:
 *
 * ```c
 * static void my_foo_cb(char* action, char* payload, void *userdata) {
 *   LOG(LL_INFO, ("Crontab foo job fired! Payload: %.*s", payload.len, payload.p));
 *   (void) action;
 *   (void) userdata;
 * }
 *
 * // Somewhere else:
 * crontab_register_handler("foo", my_foo_cb, NULL);
 * ```
 *
 * The code above maps action `foo` in the JSON to the callback `my_foo_cb`.
 */
void crontab_register_handler(
    crontab* crontab,
    char* action, 
    crontab_cb cb,
    void *userdata
);

int crontab_get_job_count(crontab* crontab);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CRONTAB_H_ */
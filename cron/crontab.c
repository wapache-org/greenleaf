#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "json.h"

#include "crontab.h"

/* TODO(dfrank): make it configurable */
#define JSON_PATH "crontab.json"

#define MIN_ID (1)

#define MAX_FREE_JOBS_CNT 3

int crontab_job_trigger_default( crontab_job* job, void *user_data){
    time_t now; time(&now);
    
    if(job->next_trigger_time==0){
      if(job->trigger_on_load){
        job->next_trigger_time = now;
      }else{
        job->next_trigger_time = (*job->timer)(job->cron_expr, now);
      }
    }

    if(now >= job->next_trigger_time){ // 如果当前时间>下一次执行时间, 那么触发执行
        job->last_trigger_time = now;
        job->next_trigger_time = (*job->timer)(job->cron_expr, job->last_trigger_time);

        (*job->runner)(job->id,job->name,job->action,job->payload,user_data);
    }
    return 0;
};

int crontab_job_runner_default(
    const int id,
    const char* name,
    const char* action, 
    const char* payload,
    void *userdata
){
    printf("job #%d triggered: name=%s, action=%s, payload=%s\n", id, name, action, payload);

    return 0;
};


int crontab_new(crontab** cron_tab)
{
  crontab* tab = calloc(1, sizeof(crontab));
  if(tab==NULL) return 1;

  tab->path = JSON_PATH;
  tab->job_size = 1024;
  tab->jobs = calloc(tab->job_size, sizeof(crontab_job*));
  if(tab->jobs==NULL) {
    crontab_free(tab);
    return 2;
  }

  *cron_tab = tab;

  return 0;
}

int crontab_free(crontab* crontab)
{
  if(!crontab) return 1;
  
  // 有可能这个值不是指向堆内存, 所以不自动释放
  // free(crontab->path);

  for (size_t i = 0; i < crontab->job_size; i++)
  {
    crontab_free_job((crontab_job*) (crontab->jobs+i));
  }
  
  memset(crontab, 0, sizeof(crontab)); // 调试代码时用的, 生产环境不需要

  free(crontab);

  return 0;
}

/*
 * Reads crontab from the given file, and adds the jobs to cron library.
 * If NULL is given as json_path, default crontab.json path will be used.
 */
int crontab_load(crontab* crontab)
{
  // TODO 根据crontab->path读取文件内容, 并解析
}

/*
 * Writes crontab to the given file, and adds the jobs to cron library.
 * If NULL is given as json_path, default crontab.json path will be used.
 */
int crontab_save(crontab* crontab)
{
  // TODO 将crontab保存到crontab->path文件
}

int crontab_new_job(crontab_job** job)
{
  crontab_job* cj = calloc(1, sizeof(crontab_job));
  if(cj==NULL) return 1;

  cj->enable = 1;

  cj->timer = cronexpr_next;
  cj->trigger = crontab_job_trigger_default;
  cj->runner = crontab_job_runner_default;

  *job = cj;

  return 0;
}

int crontab_free_job(crontab_job* job)
{
  if(!job) return 1;

  cronexpr_free(job->cron_expr);
  free(job->action);
  free(job->payload);

  memset(job, 0, sizeof(job)); // 调试代码时用的, 生产环境不需要

  free(job);

  return 0;
}

int crontab_add_job(crontab* crontab, crontab_job* job)
{
  for (size_t i = 0; i < crontab->job_size; i++)
  {
    crontab_job* j = crontab->jobs[i];
    if(!j) {
      job->id = crontab->next_id++;
      j = job;
      return 0;
    }
  }

  // TODO 自动扩容

  return 1; // job到达了上限, 无法添加了
}

/*
 * Remove a job by its id.
 *
 * Returns true in case of success, false otherwise.
 *
 * If `perr` is not NULL, the error message will be written there (or NULL
 * in case of success). The caller should free the error message.
 */
int crontab_remove_job(crontab* crontab, crontab_job* job)
{
  for (size_t i = 0; i < crontab->job_size; i++)
  {
    crontab_job* j = crontab->jobs[i];
    if(j && j==job) {
      j = NULL; // 需要手工调用crontab_free_job释放job
      return 0;
    }
  }
  return 1; // 找不到job
}

/*
 * Deactivate all existing cron jobs
 */
int crontab_clear_jobs(crontab* crontab)
{
  for (size_t i = 0; i < crontab->job_size; i++)
  {
    crontab_job* job = crontab->jobs[i];
    if(job) {
      crontab_free_job(job); // 不需要手工调用crontab_free_job释放job
      job = NULL;
      return 0;
    }
  }
}

int crontab_get_job(
    crontab_job* job, 
    char **perr
){


  return 1; // 未实现
}

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
  void *user_data
){
  for (size_t i = 0; i < crontab->job_size; i++)
  {
    crontab_job* job = crontab->jobs[i];
    if(job) 
      if(cb(job, user_data))
        return 1;
  }
  return 0; // 未实现
}

void crontab_register_handler(
  crontab* crontab,
  char* action, 
  crontab_cb cb,
  void *userdata
){
  // 未实现
}

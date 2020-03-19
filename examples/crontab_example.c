#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>

#include "crontab.h"
#include "common/logger.h"

char* new_string(char* content)
{
    size_t len = strlen(content);
    char* str = malloc(len+1);
    if(str){
        memcpy(str, content, len);
        *(str+len) = '\0';
    }
    return str;
}

int main(int argc, char* argv[])
{
    logger_info("the crontab example is running.");

    const char* err;

    crontab* crontab = NULL;

    if(crontab_new(&crontab)){
        goto free;
    }


    { // 测试添加和删除
        crontab_job* job = NULL;

        if(crontab_new_job(&job)){
            goto free;
        }

        job->enable = true;
        job->name = new_string("job1");
        job->action = new_string("pg_vacuum");
        job->payload = new_string("sys_user,sys_role");
        job->cron_expr = cronexpr_parse("0 0 0 * * *", &err);

        if(crontab_add_job(crontab, job)){
            goto free_job1;
        }

        if(crontab_remove_job(crontab, job)){
           goto free_job1;
        }
        
        free_job1:
            crontab_free_job(job);
            job = NULL;
    }
    
    // { // 测试遍历

    //     if(crontab_add_job(crontab, job)){
    //         goto free;
    //     }

    //     if(crontab_add_job(crontab, job)){
    //         goto free;
    //     }

    //     if(crontab_add_job(crontab, job)){
    //         goto free;
    //     }
        
    // }

    // { // 测试保存和装载

    //     if(crontab_save(crontab)){
    //         goto free;
    //     }

    //     if(crontab_clear_jobs(crontab)){
    //         goto free;
    //     }

    //     if(crontab_load(crontab)){
    //         goto free;
    //     }
    // }


    {
        crontab_job* job = NULL;

        if(crontab_new_job(&job)){
            goto free;
        }

        job->name = new_string("job1");
        job->action = new_string("pg_vacuum");
        job->payload = new_string("sys_user,sys_role");
        job->cron_expr = cronexpr_parse("0/3 * * * * *", &err);

        if(crontab_add_job(crontab, job)){
            goto free;
        }

        while(true){
            crontab_iterate(crontab, crontab_job_trigger_default, NULL);
            sleep(1);
        }

    }

free:
    crontab_free(crontab);

    return 0;
}





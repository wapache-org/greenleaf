#include<stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>

// 执行下面这个函数, 会出入3行信息.
int fork_1(){

	printf("it's the main process step 1!!\n\n");

	fork(); // 分叉, 一生二, 同时运行

	printf("step2 after fork() !!\n\n"); // 父进程和子进程都会执行这条语句

	int i; 
  scanf("%d",&i);   // prevent exiting, 用scanf来防止两个进程退出
	return 0;
}

// fork的目标就是让两个进程执行不同的东西, 像fork_1这种, 执行一样的代码不是我们想要的
int fork_2(){

	int i;
    pid_t childpid = fork();
	if (childpid == 0){
		//child process
		for (i=1; i<=8; i++){
			printf("This is child process\n");
		}
	}else if(childpid>0){
		//parent process
		for(i=1; i<=8; i++){
			printf("This is parent process, child process id is: %d\n", childpid);
		}
	}else{
        // fork失败...
    }

	printf("step2 after fork() !!\n\n");// 但是注意, 这里还是会两个进程都执行...
}

// 不想像fork_2那样, 也很简单, 价格exit, 让它提前结束即可
int fork_3(){
	
	int i;
    pid_t childpid = fork();
	if (childpid == 0){
		//child process
		for (i=1; i<=8; i++){
			printf("This is child process, parent process id is: %d, child process id is: %d\n", getppid(), getpid());
		}
		exit(0);   // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> here
	}else{
		//parent process
		for(i=1; i<=8; i++){
			printf("This is parent process, parent process id is: %d, child process id is: %d\n", getpid(), childpid);
		}
	}

	printf("step2 after fork() !!\n\n");
}

// 如果不想两个进程并行执行, 那也很简单, 主进程等待一下即可
int fork_4(){

	int i;
    pid_t childpid = fork();
	if (childpid == 0){
		//child process
		for (i=1; i<=8; i++){
			printf("This is child process, parent process id is: %d, child process id is: %d\n", getppid(), getpid());
		}
		exit(0);   // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> here
	}else{
		//parent process

        prctl(PR_SET_PDEATHSIG,SIGHUP); // 防止父进程意外退出, 子进程成了野生进程

        int status;
        waitpid(childpid,&status,0);
        
		for(i=1; i<=8; i++){
			printf("This is parent process, parent process id is: %d, child process id is: %d\n", getpid(), childpid);
		}
	}

	printf("step2 after fork() !!\n\n");
}


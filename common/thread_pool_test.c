/* 
 * WHAT THIS EXAMPLE DOES
 * 
 * We create a pool of 4 threads and then add 40 tasks to the pool(20 task1 
 * functions and 20 task2 functions). task1 and task2 simply print which thread is running them.
 * 
 * As soon as we add the tasks to the pool, the threads will run them. It can happen that 
 * you see a single thread running all the tasks (highly unlikely). It is up the OS to
 * decide which thread will run what. So it is not an error of the thread pool but rather
 * a decision of the OS.
 * 
 * */

#include <stdio.h>
#include <pthread.h>
#include "thread_pool.h"


void task1(){
	printf("Thread #%u working on task1\n", (int)pthread_self());
}


void task2(){
	printf("Thread #%u working on task2\n", (int)pthread_self());
}


int main(){
	
	puts("Making thread pool with 4 threads");
	threadpool thpool = thpool_init("thread-pool", 4);

	puts("Adding 40 tasks to thread pool");
	int i;
	for (i=0; i<20; i++){
		thpool_add_work(thpool, (void*)task1, NULL);
		thpool_add_work(thpool, (void*)task2, NULL);
	};

	puts("Killing threadpool");
	thpool_destroy(thpool);
	
	return 0;
}
#ifndef __THREAD_POOL_H_
#define __THREAD_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

#define DEFAULT_CIRCULAR_TIME               5            /*master node manage time,every DEFAULT_CIRCULAR_TIME detecting the pool*/
#define MIN_WAIT_TASK_NUMS                  1            /*The pool alive the minimal thread for task*/
#define DEFAULT_THREAD_NUMS_CREATE_DESTROY  1            /*Create/Destroy threads default num for every time*/
#define True                                1
#define False                               0
typedef unsigned int                        u32;
typedef struct ThreadPool ThreadPool_t;
//  free the threadpool
u32 thread_pool_free(ThreadPool_t *pool_ptr);
// master thread function
void *master_thread_for_pool(void *threadpool_p);
// destroy thread pool
u32 thread_pool_destroy(ThreadPool_t *pool_ptr);
// every thread in pool queue do function
void *thread_pool_thread_work(void *threadpool_ptr);
// the interface for user to add task to the pool
u32 thread_pool_add_task(ThreadPool_t *pool, void *(*function)(void *arg), void *arg);
u32 judge_thread_is_alive(pthread_t tid);
// create thread pool
ThreadPool_t *thread_pool_create(u32 min_thread_num, u32 max_thread_num, u32 thread_queue_max_size);
#endif
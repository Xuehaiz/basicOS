#include "thread_pool.h"
typedef struct {
   void *(*function)(void *);                               /*Function pointer for call function*/
   void *arg;
} threadpool_task_queue_t;
typedef struct ThreadPool_Info{
   /*thread pool info*/
   u32 min_thread_num;                                      /*The minimal thread nums in this pool*/
   u32 max_thread_num;                                      /*The max thread nums in this pool*/
   u32 live_thread_num;                                     /*The alive thread nums in this pool*/
   u32 work_thread_num;                                     /*The working thread nums in this pool*/
   u32 wait_kill_thread_num;                                /*The thread need to kill/destroy nums*/
}ThreadPool_Info_t;
typedef struct TaskQueue_Info{
    /*Task queue info*/
    u32 queue_head;                                         /*queue*/
    u32 queue_tail;                                         /*tail*/
    u32 queue_size_current_time;                            /*queue size in current time*/
    u32 queue_max_size;                                     /*The max task nums in this queue*/
}TaskQueue_Info_t;
struct ThreadPool{
    ThreadPool_Info_t   thread_pool_info;
    TaskQueue_Info_t    taskqueue_info;

    /*pthread mutex and condition info*/ 
    pthread_mutex_t lock_thread_pool;                       /*lock the pool struct*/
    pthread_mutex_t lock_work_num;                          /*Mutex for work num*/
    pthread_cond_t  task_queue_not_full_cond;               /*Task queue not full condition variable*/
    pthread_cond_t  task_queue_not_empty_cond;              /*Task queue not empty condition variable*/
    pthread_t *threads_ids;                                 /*For threads id,this is a array*/
    pthread_t master_tid;                                   /*Master thread id for linux kernel not for POSIX*/
    threadpool_task_queue_t *task_queue;                    /*Task Queue*/

    u32 Pool_Status_RunOrDown;                              /*True Pool will destroy,False is working*/
};
ThreadPool_t *thread_pool_create(u32 min_thread_num, u32 max_thread_num, u32 thread_queue_max_size)
{
    ThreadPool_t    *pool=NULL;
    if((pool=(ThreadPool_t *)malloc(sizeof(ThreadPool_t)))==NULL)
    {
        fprintf(stderr,"Malloc for thread pool error%s\n","=");
        return NULL;
    }
    /*First initial thread pool info and task queue info*/ 
    pool->thread_pool_info.min_thread_num=min_thread_num;
    pool->thread_pool_info.max_thread_num=max_thread_num;
    pool->thread_pool_info.live_thread_num=min_thread_num;
    pool->thread_pool_info.work_thread_num=0;
    pool->thread_pool_info.wait_kill_thread_num=0;
    pool->taskqueue_info.queue_head=0;
    pool->taskqueue_info.queue_tail=0;
    pool->taskqueue_info.queue_size_current_time=0;
    pool->taskqueue_info.queue_max_size=thread_queue_max_size;
    pool->Pool_Status_RunOrDown=False;
    /*Second inital thread space in heap*/
    if((pool->threads_ids=(pthread_t *)malloc(sizeof(pthread_t)*max_thread_num))==NULL)
    {
        fprintf(stderr,"Malloc for the max threads thread pool error %s\n","=");
        free(pool);
        // thread_pool_free(pool);
        return NULL;
    }
    memset(pool->threads_ids, 0, sizeof(pthread_t)*max_thread_num);
    if((pool->task_queue=(threadpool_task_queue_t *)malloc(sizeof(threadpool_task_queue_t)*thread_queue_max_size))==NULL)
    {
        fprintf(stderr,"Malloc for the max threads task queue error %s\n","=");
        free(pool);
        free(pool->threads_ids);
        return NULL;
    }
    memset(pool->task_queue, 0, sizeof(threadpool_task_queue_t)*thread_queue_max_size);
    /*Third initial the mutex info and condition info*/
    if(pthread_mutex_init(&(pool->lock_thread_pool), NULL) != 0)
    {
        fprintf(stderr,"pthread_mutex_init for thread pool error %s\n","=");
        thread_pool_free(pool);
        return NULL;
    }
    if(pthread_mutex_init(&(pool->lock_work_num), NULL) != 0)
    {
        fprintf(stderr,"pthread_mutex_init for thread lock_work_num %s\n","=");
        thread_pool_free(pool);
        return NULL;
    }
    if(pthread_cond_init(&(pool->task_queue_not_empty_cond), NULL) != 0)
    {
        fprintf(stderr,"pthread_cond_init for thread task_queue_not_empty_cond %s\n","=");
        thread_pool_free(pool);
        return NULL;
    }   
    if(pthread_cond_init(&(pool->task_queue_not_full_cond), NULL) != 0)
    {
        fprintf(stderr,"pthread_cond_init for thread task_queue_not_full_cond %s\n","=");
        thread_pool_free(pool);
        return NULL;
    } 
    /*Fourth Create the minimal working thread */ 
    for(int i=0;i<min_thread_num;i++)
    {
        pthread_create(&(pool->threads_ids[i]), NULL, thread_pool_thread_work, (void *)pool);
        fprintf(stdout,"Create thread Kernel Id:%ld address:0x%x\n",(long int)syscall(__NR_gettid),(unsigned int)pool->threads_ids[i]);
    }
    pthread_create(&(pool->master_tid), NULL, master_thread_for_pool, (void *)pool);
    return pool;


}
u32 thread_pool_free(ThreadPool_t *pool_ptr)
{
    if (pool_ptr == NULL)return 1;
    if(pool_ptr->task_queue)
    {
        free(pool_ptr->task_queue);
        // return 1;
    }
    if(pool_ptr->threads_ids)
    {
        free(pool_ptr->threads_ids);
        /*destroy the mutex*/ 
        pthread_mutex_trylock(&(pool_ptr->lock_work_num));
        pthread_mutex_destroy(&(pool_ptr->lock_work_num));
        pthread_mutex_trylock(&(pool_ptr->lock_thread_pool));
        pthread_mutex_destroy(&(pool_ptr->lock_thread_pool));
        pthread_cond_destroy(&(pool_ptr->task_queue_not_empty_cond));
        pthread_cond_destroy(&(pool_ptr->task_queue_not_full_cond));

    }
    free(pool_ptr);
    pool_ptr=NULL;
    return 0;
}
// Destroy the pool
u32 thread_pool_destroy(ThreadPool_t *pool_ptr)
{
    if (pool_ptr == NULL)return 1;
    pool_ptr->Pool_Status_RunOrDown=True;
    /*destroy the master thread*/
    pthread_join(pool_ptr->master_tid,NULL); 
    /*broadcast the close condition for all not busy thread*/
    /*This case occur because there are no tasks in queue and we can detecting this condition to kill thread*/ 
    for(int i=0;i<pool_ptr->thread_pool_info.live_thread_num;i++)
    {
        pthread_cond_broadcast(&(pool_ptr->task_queue_not_empty_cond));
    } 
    for(int i=0;i<pool_ptr->thread_pool_info.live_thread_num;i++)
    {
        pthread_join(pool_ptr->threads_ids[i],NULL);
    }
    thread_pool_free(pool_ptr);
    return 0;
}
void *master_thread_for_pool(void *threadpool_p)
{
    ThreadPool_t    *pool=(ThreadPool_t    *)threadpool_p;
    fprintf(stdout,"====Master node will start manage the pool and will circular ervery %d seconds!===\n",DEFAULT_CIRCULAR_TIME);
    while (!pool->Pool_Status_RunOrDown)//Not false
    {
        // fprintf(stdout,"====Master node will start manage the pool and will circular ervery %d seconds!===\n",DEFAULT_CIRCULAR_TIME);
        sleep(DEFAULT_CIRCULAR_TIME);//Delay a consitant time to check or manage the thread pool
        pthread_mutex_lock(&(pool->lock_thread_pool));
        // u32 live_thread_nums=0;
        // u32 l/ive_queue_task_nums=0;
        u32 live_thread_nums=pool->thread_pool_info.live_thread_num;
        u32 live_queue_task_nums=pool->taskqueue_info.queue_size_current_time;
        pthread_mutex_unlock(&(pool->lock_thread_pool));
        pthread_mutex_lock(&(pool->lock_work_num));
        // u32 work_thread_nums=0;
        u32 work_thread_nums=pool->thread_pool_info.work_thread_num;
        pthread_mutex_unlock(&(pool->lock_work_num));
        /* code */
        fprintf(stdout,"====The Master Node: %ld collecting the pool info show as:live thread:%d-live task queue:%d-live work thread:%d====\n",(long int)syscall(__NR_gettid),live_thread_nums,live_queue_task_nums,work_thread_nums);
        /*Case1:create the more threads if queue has more than the minimal wait thread in the pool*/ 
        if(live_queue_task_nums>=MIN_WAIT_TASK_NUMS && live_thread_nums<=pool->thread_pool_info.max_thread_num)
        {
            fprintf(stdout,"====The Master Node:%ld will add not more than the max thread in pool====\n",(long int)syscall(__NR_gettid));
            pthread_mutex_lock(&(pool->lock_thread_pool));
            u32 count=0;
            for(int i=0;i<pool->thread_pool_info.max_thread_num && count<=DEFAULT_THREAD_NUMS_CREATE_DESTROY && pool->thread_pool_info.live_thread_num<=pool->thread_pool_info.max_thread_num;i++)
            {
                if(pool->threads_ids[i]==0 || !(judge_thread_is_alive(pool->threads_ids[i])))
                {
                    fprintf(stdout,"====Master Node Create the new thread START =====%s\n","=");
                    pthread_create(&(pool->threads_ids[i]),NULL,thread_pool_thread_work,(void *)pool);
                    count++;
                    pool->thread_pool_info.live_thread_num++;
                    fprintf(stdout,"====Master Node Create the new thread END =====%s\n","=");
                }
            }
            pthread_mutex_unlock(&(pool->lock_thread_pool));            
        }
        /*Case 2:Clear the more thread. Because working thread less than the live thread and live threads are more than the minimal wait thread in the pool*/
        if((work_thread_nums)<live_thread_nums && live_thread_nums>pool->thread_pool_info.min_thread_num) 
        {
            pthread_mutex_lock(&(pool->lock_thread_pool));
            pool->thread_pool_info.wait_kill_thread_num=DEFAULT_THREAD_NUMS_CREATE_DESTROY;
            pthread_mutex_unlock(&(pool->lock_thread_pool));
            fprintf(stdout,"====Master Node:%ld Will clear the default thread because of the task queue is zero or task less than the minimal wait thread =====\n",(long int)syscall(__NR_gettid));

            for(int i=0;i<DEFAULT_THREAD_NUMS_CREATE_DESTROY;i++)
            {
                pthread_cond_signal(&(pool->task_queue_not_empty_cond));

            }
        }
    }
    sched_yield();
    
    return NULL;
}
u32 judge_thread_is_alive(pthread_t tid)
{
    if(pthread_kill(tid,0)==ESRCH)
    {
        return False;
    }
    return True;
}
void *thread_pool_thread_work(void *threadpool_ptr)
{
    ThreadPool_t    *pool=(ThreadPool_t    *)threadpool_ptr;
    threadpool_task_queue_t taskqueue_t;
    fprintf(stdout,"====Create Thread ID:%lu  ======\n",(long int)syscall(__NR_gettid));
    while (True)
    {
        pthread_mutex_lock(&(pool->lock_thread_pool));

        /*First,We need to wait kill signal to kill the thread from master node*/ 
        /*The thread will wait here if no task in the queue*/ 
        while ((pool->taskqueue_info.queue_size_current_time==0) && !(pool->Pool_Status_RunOrDown))
        {
            fprintf(stdout,"====The POOL thread kernel ID:%lu WAIT ======\n",(long int)syscall(__NR_gettid));
            pthread_cond_wait(&(pool->task_queue_not_empty_cond), &(pool->lock_thread_pool));
            //we need delete the thread which is in the wait list
            if(pool->thread_pool_info.wait_kill_thread_num>0)
            {
                pool->thread_pool_info.wait_kill_thread_num--;
                if(pool->thread_pool_info.live_thread_num > pool->thread_pool_info.min_thread_num)
                {
                    fprintf(stdout,"====The POOL thread kernel ID:%lu WILL BE CLOSED ======\n",(long int)syscall(__NR_gettid));
                    pool->thread_pool_info.live_thread_num--;
                    pthread_mutex_unlock(&(pool->lock_thread_pool));
                    pthread_exit(NULL);//Direct exit the thread
                }
            }
            /* code */
        }
        /*Second,we need to judge the pool status*/ 
        if(pool->Pool_Status_RunOrDown)
        {
            // pthread_mutex_lock(&(pool->lock_thread_pool));   
            pthread_mutex_unlock(&(pool->lock_thread_pool));
            fprintf(stdout,"====The POOL thread kernel ID:%lu WILL BE CLOSED,Pool is shutdown ======\n",(long int)syscall(__NR_gettid));
            // pool->thread_pool_info.live_thread_num--;

            
            pthread_exit(NULL);
        }
        /*Third,We can get task from the queue*/ 
        taskqueue_t.function=pool->task_queue[pool->taskqueue_info.queue_head].function;
        taskqueue_t.arg=pool->task_queue[pool->taskqueue_info.queue_head].arg;
        pool->taskqueue_info.queue_head=(pool->taskqueue_info.queue_head+1) % pool->taskqueue_info.queue_max_size;
        pool->taskqueue_info.queue_size_current_time--;
        /*Add one more task */
        pthread_cond_signal(&(pool->task_queue_not_full_cond));
        pthread_mutex_unlock(&(pool->lock_thread_pool));
        /*Fourth,Execute the task function*/
        pthread_mutex_lock(&(pool->lock_work_num));
        pool->thread_pool_info.work_thread_num++;
        pthread_mutex_unlock(&(pool->lock_work_num));
        // execute the task
        fprintf(stdout,"====The thread kernel ID:%lu Execute Start ======\n",(long int)syscall(__NR_gettid));
        (*(taskqueue_t.function))(taskqueue_t.arg);
        fprintf(stdout,"====The thread kernel ID:%lu Execute End ======\n",(long int)syscall(__NR_gettid));
        pthread_mutex_lock(&(pool->lock_work_num));
        pool->thread_pool_info.work_thread_num--;
        pthread_mutex_unlock(&(pool->lock_work_num));                    
        /* code */
    }
    pthread_exit(NULL);
    
}
u32 thread_pool_add_task(ThreadPool_t *pool, void *(*function)(void *arg), void *arg)
{
    pthread_mutex_lock(&(pool->lock_thread_pool));
    /*First case,The task queue is full,Task need to wait the free thread to do this*/ 
    while ((pool->taskqueue_info.queue_size_current_time==pool->taskqueue_info.queue_max_size) && !(pool->Pool_Status_RunOrDown))
    {
        fprintf(stdout,"===The task queue is full,Task need to wait the free thread to do this==%s\n","=");
        pthread_cond_wait(&(pool->task_queue_not_full_cond),&(pool->lock_thread_pool));
        /* code */
    }
    /*Second,execpt the pool is close*/
    if(pool->Pool_Status_RunOrDown)
    {
        fprintf(stdout,"===The pool is closed You can not create/add a new task==%s\n","=");
        pthread_mutex_unlock(&(pool->lock_thread_pool));
        return 1;
    } 
    if(pool->task_queue[pool->taskqueue_info.queue_tail].arg!=NULL)
    {
        free(pool->task_queue[pool->taskqueue_info.queue_tail].arg);
        pool->task_queue[pool->taskqueue_info.queue_tail].arg = NULL;
    }
    fprintf(stdout,"=====Add the new task to the queue====%s\n","=");
    pool->task_queue[pool->taskqueue_info.queue_tail].function=function;
    pool->task_queue[pool->taskqueue_info.queue_tail].arg=arg;
    pool->taskqueue_info.queue_tail=(pool->taskqueue_info.queue_tail+1) % pool->taskqueue_info.queue_max_size;
    pool->taskqueue_info.queue_size_current_time++;
    // fprintf(stdout,"=====Send awake signal to master to awake one thread====%s\n","=");
    pthread_cond_signal(&(pool->task_queue_not_full_cond));
    pthread_mutex_unlock(&(pool->lock_thread_pool));
    return 0;
}

// typedef struct threadargs {
//   int	id;
// }threadargs_t;
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// int num=0;
// void *print_test()
// {
//     pthread_mutex_lock(&mutex);
//     printf("HH this is num:%d  test thread:%lu\n",num,(long int)syscall(__NR_gettid));//pthread_self,knerl,POXI
//     num++;
//     pthread_mutex_unlock(&mutex);
//     // return 0;
// }
// int main()
// {
//     threadargs_t argsp;
//     argsp.id=10;
//     ThreadPool_t *thp = thread_pool_create(10, 100, 100);
//     fprintf(stdout,"======threadpool init ... ... ======\n");

//    for(int i=0;i<90;i++)
//    {
//     thread_pool_add_task(thp, print_test, NULL);
//    }
//     sleep(100);
//    thread_pool_destroy(thp);

//     return 0;
// }
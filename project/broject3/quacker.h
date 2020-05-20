/*=============================================================================
 * Program Name: topicStore.c
 * Author: Xuehai Zhou
 * Date: May 26, 2020
 * Description:
 *     CIS 415 Project 3 InstaQuack Part 2~5 source file
 *
 * Notes:
 *     N/A
 *===========================================================================*/

#ifndef QUACKER_H_
#define QUACKER_H_

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "topicStore.h"

#define NUMPROXIES 5
#define MAXPUBS 20
#define MAXSUBS 20
#define TRUE 1

int condition = 1;

typedef struct threadPool {
    // int numFiles;
    char filename[FILENAME_MAX];
    int thread_idx;
    pthread_t thread;
    int isFree;
} threadPool;

pthread_t cleanThread;

pthread_mutex_t mylock[NUMPROXIES]; // = PTHREAD_MUTEX_INITIALIZER;

int initPool(threadPool *myPool);

int initLock();

void *publisher(void *voidPool);

void *subscriber(void *voidPool);

int pubParse(pthread_mutex_t mylock, char *filename);

int subParse(pthread_mutex_t mylock, char *filename);

void *clean(void *voidPool);

int joinPool(threadPool *myPool);

int destroyLock();

#endif
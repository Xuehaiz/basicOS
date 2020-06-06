/*=============================================================================
 * Program Name: qucker.h
 * Author: Xuehai Zhou
 * Date: May 26, 2020
 * Description:
 *     CIS 415 Project 3 InstaQuack Part 2~5 header file
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

#define NUMPROXIES 2
#define MAXPUBS 20
#define MAXSUBS 20
#define TRUE 1

int condition = 1;

typedef struct threadPool {
    char filename[FILENAME_MAX];
    int thread_idx;
    pthread_t thread;
    int isFree;
} threadPool;

pthread_t cleanThread;

int initPool(threadPool *myPool);

void *publisher(void *voidPool);

void *subscriber(void *voidPool);

int pubParse(char *filename);

int subParse(char *filename);

void *clean(void *voidPool);

int destroyLock();

#endif
/*=============================================================================
 * Program Name: topicStore.c
 * Author: Xuehai Zhou
 * Date: May 26, 2020
 * Description:
 *     CIS 415 Project 3 InstaQuack Part 1 source file
 *
 * Notes:
 *     N/A
 *===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "topicstore.h"

int initQueue(int qid, char *name, int len, topicQueue *myQueue) {
	newQueue->qid = qid;
    if (strlen(name) >= NAMESIZE) {
        printf("Error! Failed to build topicQueue. Queue name too long.\n");
        return 0;
    }
    strcpy(newQueue->name, name);
    newQueue->length = len;
    newQueue->entryCtr = 0;
    newQueue->head = 0;
    newQueue->tail = 0;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&newQueue->mylock, &attr);
    pthread_mutexattr_destroy(&attr);

    return 1;
}

int destroyTQ(topicQueue *TQ) {
    if (!(pthread_mutex_destroy(&TQ->lock) == 0)) {
    	fprintf(stderr, "Error! Destroy queue failed%s\n");
    	return 0;
    }
    return 1;
}

int enqueue(topicEntry *newEntry, topicQueue *TQ) {
	int is_full = 0;
	int ret = 0;

	pthread_mutex_lock(&(TQ->mylock));

	int head = TQ->head;
	int tail = TQ->tail;

	if (((tail + 1) % TQ->length) == head) { // buffer is full
		ret = 0;
	}
	else {
		TQ->buffer[tail].entryNum = TQ->entryCtr;
		TQ->entryCtr++;
		TQ->buffer[tail] = newEntry;
		gettimeofdat(&TQ->buffer[tail].timeStamp, NULL);
		ret = 1;
	}


}

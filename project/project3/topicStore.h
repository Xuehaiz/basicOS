/*=============================================================================
 * Program Name: topicStore.h
 * Author: Xuehai Zhou
 * Date: May 26, 2020
 * Description:
 *     CIS 415 Project 3 InstaQuack Part 1 header file
 *
 * Notes:
 *     N/A
 *===========================================================================*/

#ifndef TOPICSTORE_H_
#define TOPICSTORE_H_

#include <stdio.h>
#include <pthread.h>

#define NAMESIZE 20
#define MAXENTRIES 100
#define MAXTOPICS 20
#define URLSIZE 256
#define CAPSIZE 256

typedef struct topicEntry {
    int entryNum;
    struct timeval timeStamp;
    long int pubID;
    char photoURL[URLSIZE]; // URL to photo
    char photoCaption[CAPSIZE]; // photo caption
} topicEntry;

typedef struct topicQueue {
    int qid;
    char name[NAMESIZE];
    int entryCtr;
    int length;
    topicEntry buffer[MAXENTRIES];
    int head;
    int tail;
    pthread_mutex_t mylock;
} topicQueue;

typedef struct topicStore {
    int numTopics;
    topicQueue topics[MAXTOPICS];
} topicStore;

topicEntry initEntry(long int pubID, char *photoURL, char *photoCaption);

int initQueue(int qid, char *name, int len, topicQueue *myQueue);

int enqueue(topicEntry *newEntry, topicQueue *TQ);

int dequeue(topicEntry *TE, topicQueue *TQ);

int getEntry(int lastEntry, topicQueue *TQ, topicEntry *TE);

#endif


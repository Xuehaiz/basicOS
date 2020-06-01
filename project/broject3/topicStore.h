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
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define URLSIZE 256
#define CAPSIZE 256
#define NAMESIZE 20
#define MAXENTRIES 100
#define MAXTOPICS 20

typedef struct topicEntry {
    int entryNum;
    struct timeval timeStamp;
    int pubID;
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


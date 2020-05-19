/*
Description:
    CIS 415 Project 3 InstaQuack!
    topicstore.c

Author: Joseph Goh
Last updated: 2019/12/08

Notes:
    N/A

TO DO:
    N/A
*/

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include "topicstore.h"

int buildTQ(int id, char *name, int length, topicQueue *newQueue) {
    newQueue->id = id;
    if (strlen(name) >= NAMESIZE) {
        printf("ERROR: Failed to build topicQueue. Name too long.\n");
        return 0;
    }
    strcpy(newQueue->name, name);
    newQueue->length = length;
    // Queue starts out empty
    newQueue->entryCtr = 1;
    newQueue->head = -1;
    newQueue->tail = 0;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&newQueue->lock, &attr);
    pthread_mutexattr_destroy(&attr);

    return 1;
}

int destroyTQ(topicQueue *TQ) {
    pthread_mutex_destroy(&TQ->lock);
    return 1;
}

int enqueue(topicEntry *newEntry, topicQueue *TQ) {
    int is_full = 0;
    int ret;

    pthread_mutex_lock(&TQ->lock);
    
    int head = TQ->head;
    int tail = TQ->tail;

    // Check whether queue is full and set appropriate tail value
    // Case 1: topicQueue is empty
    if (head == -1) {
        TQ->head = 0;
        TQ->tail = 0;
    }
    // Case 2: topicQueue is NOT full (incrementing the tail wouldn't overwrite the head)
    else if ((tail + 1) % TQ->length == head) {
        if (tail + 1 == TQ->length) {
            TQ->tail = 0;
        }
        else {
            TQ->tail++;
        }
    }
    // Case 3: topicQueue is full (since it's not empty or not-full)
    else {
        is_full = 1;
    }

    // enqueue the topic entry into buffer at updated tail index unless the queue is full
    if (is_full) {
        ret = 0;
    }
    else {
        TQ->buffer[TQ->tail].entryNum = TQ->entryCtr;
        TQ->entryCtr++;
        gettimeofday(&TQ->buffer[TQ->tail].timeStamp, NULL);
        TQ->buffer[TQ->tail].pubID = newEntry->pubID;
        strcpy(TQ->buffer[TQ->tail].photoURL, newEntry->photoURL);
        strcpy(TQ->buffer[TQ->tail].photoCaption, newEntry->photoCaption);

        ret = 1;
    }

    pthread_mutex_unlock(&TQ->lock);
    return ret;
}

int getEntry(int lastEntry, topicQueue *TQ, topicEntry *entry) {
    int found = 0;
    int numEntries, index, i, ret;

    pthread_mutex_lock(&TQ->lock);

    if (TQ->head <= TQ->tail) {
        numEntries = TQ->tail - TQ->head + 1;
    }
    else {
        numEntries = TQ->tail - TQ->head + TQ->length + 1;
    }
    
    // Case 1: topicQueue is empty
    if (TQ->head == -1) {
        ret = 0;
    }
    else {
        for (i = 0; i < numEntries; i++) {
            index = (TQ->head + i) % TQ->length;
            if (TQ->buffer[index].entryNum == lastEntry + 1) {
                found = 1;
                break;
            }
        }

        // Case 2: The next entry to get is found in the topicQueue
        if (found) {
            entry->entryNum = TQ->buffer[index].entryNum;
            entry->timeStamp = TQ->buffer[index].timeStamp;
            entry->pubID = TQ->buffer[index].pubID;
            strcpy(entry->photoURL, TQ->buffer[index].photoURL);
            strcpy(entry->photoCaption, TQ->buffer[index].photoCaption);

            ret = 1;
        }
        // Case 3: topicQueue is not empty but next entry is not found...
        else {
            for (i = 0; i < numEntries; i++) {
                index = (TQ->head + i) % TQ->length;
                if (TQ->buffer[index].entryNum > lastEntry + 1) {
                    found = 1;
                    break;
                }
            }

            // Case 3-ii: The lastEntry + 1 must have been dequeued but we have an entry added after that
            if (found) {
                entry->entryNum = TQ->buffer[index].entryNum;
                entry->timeStamp = TQ->buffer[index].timeStamp;
                entry->pubID = TQ->buffer[index].pubID;
                strcpy(entry->photoURL, TQ->buffer[index].photoURL);
                strcpy(entry->photoCaption, TQ->buffer[index].photoCaption);

                ret = TQ->buffer[index].entryNum;
            }
            // Case 3-i: Our desired entry is yet to arrive
            else {
                ret = 0;
            }
        }
    }

    pthread_mutex_unlock(&TQ->lock);
    return ret;
}

int dequeue(topicQueue *TQ, suseconds_t delta) {
    struct timeval currenttime;
    long int age;
    int numEntries, index, i, ret;

    pthread_mutex_lock(&TQ->lock);

    if (TQ->head <= TQ->tail) {
        numEntries = TQ->tail - TQ->head + 1;
    }
    else {
        numEntries = TQ->tail - TQ->head + TQ->length + 1;
    }
    
    // If topicQueue is empty, we don't dequeue anything
    if (TQ->head == -1) {
        ret = 0;
    }
    else {
        // Parsing the entries from oldest to newest...
        for (i = 0; i < numEntries; i++) {
            index = (TQ->head + i) % TQ->length;
            gettimeofday(&currenttime, NULL);
            age = (long int) (currenttime.tv_sec - TQ->buffer[index].timeStamp.tv_sec) * 1000000 + (long int) (currenttime.tv_usec - TQ->buffer[index].timeStamp.tv_usec);
            // If we hit an entry that's not old enough, all entries after that aren't either so break
            if ((suseconds_t) age < delta) {
                break;
            }
        }

        // Case 1: The head (and every other entry by extension) is too new to be dequeued
        if (i == 0) {
            ret = 0;
        }
        // Case 2: Every entry is old enough to dequeue (since the for loop iterated past the tail)
        else if (i == numEntries) {
            TQ->head = -1;
            TQ->tail = 0;

            ret = 1;
        }
        // Case 3: Dequeue up to [index - 1] by updating the head to index
        else {
            TQ->head = index;

            ret = 1;
        }
    }

    pthread_mutex_unlock(&TQ->lock);
    return ret;
}

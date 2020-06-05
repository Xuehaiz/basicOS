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
#include <sys/time.h>
#include <string.h>

#include "topicStore.h"

topicQueue topic;

topicEntry initEntry(long int pubID, char *photoURL, char *photoCaption) {
	topicEntry myEntry;
	myEntry.pubID = pubID;
	strcpy(myEntry.photoURL, photoURL);
	strcpy(myEntry.photoCaption, photoCaption);
	return myEntry;
}

int initQueue(int qid, char *name, int len, topicQueue *myQueue) {
	myQueue->qid = qid;
    if (strlen(name) >= NAMESIZE || len > MAXTOPICS) {
        printf("Error! Failed to build topicQueue. Queue name or length is too long.\n");
        return 0;
    }
    strcpy(myQueue->name, name);
    myQueue->length = len;
    myQueue->entryCtr = 0;
    myQueue->head = 0;
    myQueue->tail = 0;
    myQueue->isfull = 0;
    myQueue->isempty = 1;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&myQueue->mylock, &attr);
    pthread_mutexattr_destroy(&attr);
    return 1;
}

int enqueue(topicEntry *newEntry, topicQueue *TQ) {
	int ret = 0;

	pthread_mutex_lock(&TQ->mylock);

	int head = TQ->head;
	int tail = TQ->tail;

	if (tail - head == -1) { // buffer is full  // (tail + 1) % TQ->length == head
		ret = 0;
	}
	else if (TQ->buffer[tail].entryNum != -1){
		TQ->buffer[tail].entryNum = TQ->entryCtr;
		TQ->entryCtr++;
		// TQ->isempty = 0;
		TQ->buffer[tail].pubID = newEntry->pubID;
        strcpy(TQ->buffer[tail].photoURL, newEntry->photoURL);
        strcpy(TQ->buffer[tail].photoCaption, newEntry->photoCaption);
		gettimeofday(&TQ->buffer[tail].timeStamp, NULL);
		if (tail == TQ->length) {
			TQ->tail = 0;
		}
		else {
			TQ->tail++;
		}
		ret = 1;
		
	}
	pthread_mutex_unlock(&TQ->mylock);
	return ret;
}

int dequeue(topicEntry *TE, topicQueue *TQ) {
	int ret = 0;

	pthread_mutex_lock(&TQ->mylock);

	int head = TQ->head;
	int tail = TQ->tail;
	if (head == tail) { // is empty 
		ret = 0;
	}
	else {
		TE->entryNum = TQ->buffer[head].entryNum;
		TE->timeStamp = TQ->buffer[head].timeStamp;
		TE->pubID = TQ->buffer[head].pubID;
		strcpy(TE->photoURL, TQ->buffer[head].photoURL);
		strcpy(TE->photoCaption, TQ->buffer[head].photoCaption);
		// change current entry to null
		TQ->buffer[head].entryNum = -1;
		if (head == TQ->length) {
			TQ->head = 0;
		}
		else {
			TQ->head++;
		}
		ret = 1;
	}
	pthread_mutex_unlock(&TQ->mylock);
	return ret;
}

int getEntry(int lastEntry, topicQueue *TQ, topicEntry *TE) {
	int index = 0;
	int found = 0;
	int ret = 0;
	int head = TQ->head;
	int tail = TQ->tail;
	int numEntries = tail - head + 1;

	pthread_mutex_lock(&TQ->mylock);
	// Case 1: topicQueue is empty
	if (TQ->isempty) {
		ret = 0;
	}
	else {
		for (int i = 0; i < numEntries; i++) {
			index = (head + i) % TQ->length;
			if (TQ->buffer[index].entryNum == lastEntry) {
				found = 1;
				break;
			}
		}
		// Case 2: last Entry is found in topicQueue
		if (found) {
			TE->entryNum = TQ->buffer[index].entryNum;
            TE->timeStamp = TQ->buffer[index].timeStamp;
            TE->pubID = TQ->buffer[index].pubID;
            strcpy(TE->photoURL, TQ->buffer[index].photoURL);
            strcpy(TE->photoCaption, TQ->buffer[index].photoCaption);
            ret = 1;
		}
		// Case 3: topicQueue is not empty and last entry isn't found
		else {
			for (int i = 0; i < numEntries; i++) {
				index = (head + i) % TQ->length;
				if (TQ->buffer[index].entryNum > lastEntry) {
					found = 1;
					break;
				}
			}
			// Case 3-ii: the last entry has been dequeued and have an entry added afterwards
			if (found) {
				TE->entryNum = TQ->buffer[index].entryNum;
                TE->timeStamp = TQ->buffer[index].timeStamp;
                TE->pubID = TQ->buffer[index].pubID;
                strcpy(TE->photoURL, TQ->buffer[index].photoURL);
                strcpy(TE->photoCaption, TQ->buffer[index].photoCaption);
                ret = TQ->buffer[index].entryNum;
			}
			// Case 3-i: the target entry hasn't arrived
			else {
				ret = 0;
			}
		}
	}
	pthread_mutex_unlock(&TQ->mylock);
	return ret;
}

/*int main()
{
	topicEntry te1 = initEntry(0, "Taobao", "Jack Ma");
	topicEntry te2 = initEntry(1, "Tesla", "Elon Musk");
	topicEntry te3 = initEntry(2, "Amazon", "Jeff Bezos");

	initQueue(0, "Companies", 3, &topic);

	if (enqueue(&te1, &topic)) {
		printf("Enqueue te1 success\n");
	}
	else {
		printf("Enqueue te1 failed\n");
	}

	if (enqueue(&te2, &topic)) {
		printf("Enqueue te2 success\n");
	}
	else {
		printf("Enqueue te2 failed\n");
	}

	if (enqueue(&te3, &topic)) {
		printf("Enqueue te2 success\n");
	}
	else {
		printf("Enqueue te2 failed\n");
	}

	topicEntry TE;
	for (int i = 0; i < 3; i++) {
		int dequeue_check = dequeue(&TE, &topic);
		if (dequeue_check) {
			printf("EntryNum: %d, URL: %s, Caption: %s\n", TE.entryNum, TE.photoURL, TE.photoCaption);
		}
		else {
			printf("Dequeue te%d failed\n", i+1);
		}
	}

	if (enqueue(&te1, &topic)) {
		printf("Enqueue te1 success\n");
	}
	else {
		printf("Enqueue te1 failed\n");
	}

	if (enqueue(&te2, &topic)) {
		printf("Enqueue te2 success\n");
	}
	else {
		printf("Enqueue te2 failed\n");
	}

	if (enqueue(&te3, &topic)) {
		printf("Enqueue te2 success\n");
	}
	else {
		printf("Enqueue te2 failed\n");
	}

	if (getEntry(2, &topic, &TE)) {
		printf("GET ENTRY: %d --- EntryNum: %d, URL: %s, Caption: %s\n", 
			getEntry(2, &topic, &TE), TE.entryNum, TE.photoURL, TE.photoCaption);
	}
	else {
		printf("getEntry failed\n");
	}

	//topicEntry TE;
	for (int i = 0; i < 3; i++) {
		int dequeue_check = dequeue(&TE, &topic);
		if (dequeue_check) {
			printf("EntryNum: %d, URL: %s, Caption: %s\n", TE.entryNum, TE.photoURL, TE.photoCaption);
		}
		else {
			printf("Dequeue te%d failed\n", i+1);
		}
	}
 	return 0;
}*/










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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include "topicStore.h"
#include "quacker.h"

topicStore TS;

int initPool(threadPool *myPool) {
	myPool->numFiles = 0;
	for (int i = 0; i < NUMPROXIES; i++) {
		myPool->isFree[i] = 1;
	}
	return 1;
}

int initLock() {
	for (int i = 0; i < NUMPROXIES; i++) {
		pthread_mutexattr_t attr;
	    pthread_mutexattr_init(&attr);
	    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
	    pthread_mutex_init(&mylock[i], &attr);
	    pthread_mutexattr_destroy(&attr);
	}
	return 1;
}

void *publisher(void *voidPool) {
	threadPool *pubPool = (threadPool *) voidPool;
	int th_idx, file_idx;
	// find the index of current thread 
	for (th_idx = 0; th_idx < NUMPROXIES; th_idx++) {
		if (pthread_self() == pubPool->threads[th_idx]) 
			break;
	}
	// if the index cannot be found, report an error
	if (th_idx == NUMPROXIES) {
		fprintf(stderr, "Error! Publisher <%ld> cannot find the current index\n", pthread_self());
	}

	pubPool->isFree[th_idx] = 0;
	for (int i = 0; i < pubPool->numFiles; i++) {
		pubParse(mylock[th_idx], pubPool->filename[i]);
	}
	pubPool->isFree[th_idx] = 1;
	return NULL;
}

int pubParse(pthread_mutex_t mylock, char *filename) {
	FILE *fpub;
	char *line;
	size_t len;
	char *token;
	int arg_ctr = 0;
	char *arg_arr[128];
	fpub = fopen(filename, "r");
	if (fpub == NULL) {
		fprintf(stderr, "Error! Publisher <%ld> fail to open file <%s>\n", pthread_self(), filename);
	}
	useconds_t sleep_t;
	int index = 0;
	int topicID = 0;
	int success = 0;
	topicEntry myEntry;
	while (getline(&line, &len, fpub) != EOF) {
		for (int i = 0; i < arg_ctr; i++) {
			arg_arr[i] = NULL;
		}
		arg_ctr = 0;
		while ((token = strtok_r(line, " \"\r\n", &line))) {
			arg_arr[arg_ctr] = token;
			arg_ctr++;
		}
		if (strcmp(arg_arr[0], "stop") == 0) {
			fclose(fpub);
			break;
		}
		else if (strcmp(arg_arr[0], "sleep") == 0) {
			sscanf(arg_arr[1], "%d", &sleep_t);
			usleep(sleep_t);
		}
		else if (strcmp(arg_arr[0], "put") == 0) {
			sscanf(arg_arr[1], "%d", &topicID);
			// find the topics' index 
			for (index = 0; index < TS.numTopics; index++) {
				if (TS.topics[index].qid == topicID)
					break;
			}
			// if the topic index cannot find, report an error
			if (index == TS.numTopics) {
				fprintf(stderr, "Error! Publisher <%ld> cannot find topic with ID <%d>\n", pthread_self(), topicID);
				fclose(fpub);
				return 0;
			}
			myEntry = initEntry(pthread_self(), arg_arr[2], arg_arr[3]);
			pthread_mutex_lock(&mylock);
			// Try enqueue for 30 times, and sleep 100 ms in each failure attempt interval
			for (int i = 0; i < 30; i++) {
				success = enqueue(&myEntry, &TS.topics[index]);
				if (success) {
					printf("Publisher <%ld> enqueued a new entry to topic ID: <%d>\n", pthread_self(), topicID);
					break;
				}
				usleep(100);
			}
			if (!success) {
				fprintf(stderr, "Error! Publisher <%ld> failed to enqueue a new entry to topic ID: <%d>\n", pthread_self(), topicID);
			}
			pthread_mutex_lock(&mylock);
		}
		else {
			fprintf(stderr, "Error! Publisher command does not recognize\n");
			// exit(EXIT_FAILURE);
		}
		sched_yield();
	}
	return 1;
}

void *subscriber(void *voidPool) {
	threadPool *subPool = (threadPool *) voidPool;
	int th_idx, file_idx;
	// find the index of current thread 
	for (th_idx = 0; th_idx < NUMPROXIES; th_idx++) {
		if (pthread_self() == subPool->threads[th_idx]) 
			break;
	}
	// if the index cannot be found, report an error
	if (th_idx == NUMPROXIES) {
		fprintf(stderr, "Error! Subscriber <%ld> cannot find the current index\n", pthread_self());
	}

	subPool->isFree[th_idx] = 0;
	for (int i = 0; i < subPool->numFiles; i++) {
		subParse(mylock[th_idx], subPool->filename[i]);
	}
	subPool->isFree[th_idx] = 1;
	return NULL;
}

int subParse(pthread_mutex_t mylock, char *filename) {
	FILE *fsub;
	char *line;
	size_t len;
	char *token;
	int arg_ctr = 0;
	char *arg_arr[128];
	fsub = fopen(filename, "r");
	if (fsub == NULL) {
		fprintf(stderr, "Error! Subscriber <%ld> fail to open file <%s>\n", pthread_self(), filename);
	}
	useconds_t sleep_t;
	int index = 0;
	int topicID = 0;
	int entryNum = 0;
	topicEntry myEntry;

	int lastEntry[TS.numTopics];
	for (int i = 0; i < TS.numTopics; i++) {
		lastEntry[i] = 0;
	}
	while (getline(&line, &len, fsub) != EOF) {
		for (int i = 0; i < arg_ctr; i++) {
			arg_arr[i] = NULL;
		}
		arg_ctr = 0;
		while ((token = strtok_r(line, " \"\r\n", &line))) {
			arg_arr[arg_ctr] = token;
			arg_ctr++;
		}
		if (strcmp(arg_arr[0], "stop") == 0) {
			fclose(fsub);
			break;
		}
		else if (strcmp(arg_arr[0], "sleep") == 0) {
			sscanf(arg_arr[1], "%d", &sleep_t);
			usleep(sleep_t);
		}
		else if (strcmp(arg_arr[0], "get") == 0) {
			sscanf(arg_arr[1], "%d", &topicID);
			// find the topics' index 
			for (index = 0; index < TS.numTopics; index++) {
				if (TS.topics[index].qid == topicID)
					break;
			}
			// if the topic index cannot find, report an error
			if (index == TS.numTopics) {
				fprintf(stderr, "Error! Subscriber <%ld> cannot find topic with ID <%d>\n", pthread_self(), topicID);
				fclose(fsub);
				return 0;
			}
			pthread_mutex_lock(&mylock);
			// Try getEntry for 30 times, and sleep 100 ms in each failure attempt interval
			for (int i = 0; i < 30; i++) {
				entryNum = getEntry(lastEntry[index], &TS.topics[index], &myEntry);
				if (entryNum == 1) {
					printf("Subscriber <%ld> got entry <%d> from topic ID <%d>\n", pthread_self(), myEntry.entryNum, topicID);
					printf("URL: <%s> Caption: <%s>\n", myEntry.photoURL, myEntry.photoCaption);
					lastEntry[index]++;
					break;
				}
				else if (entryNum > 1) {
					printf("Subscriber <%ld> got entry <%d> from topic ID <%d>\n", pthread_self(), myEntry.entryNum, topicID);
					printf("URL: <%s> Caption: <%s>\n", myEntry.photoURL, myEntry.photoCaption);
					lastEntry[index] = entryNum;
					break;
				}
				usleep(100);
			}
			if (!entryNum) {
				fprintf(stderr, "Error! Subscriber <%ld> failed to get entry from topic ID: <%d>\n", pthread_self(), topicID);
			}
			pthread_mutex_lock(&mylock);
		}
		else {
			fprintf(stderr, "Error! Subscriber command does not recognize\n");
			// exit(EXIT_FAILURE);
		}
		sched_yield();
	}
	return 1;
}

void *clean(void *voidPool) {
	cleanPool *clnPool = (cleanPool *) voidPool;
	useconds_t delta = clnPool->delta;
	struct timeval curr_time;
	struct timeval lastclean;
	double diff_t;
	struct topicEntry TE;
	gettimeofday(&lastclean, NULL);
	int numEntries = 0;
	int th_idx = 0;
	topicEntry myEntry;
	while (condition) {
		gettimeofday(&curr_time, NULL);
		diff_t = difftime(curr_time.tv_sec, lastclean.tv_sec);
		if (diff_t > delta) {
			gettimeofday(&lastclean, NULL);
			for (int i = 0; i < TS.numTopics; i++) {
				numEntries = TS.topics[i].tail - TS.topics[i].head + 1;
				for (int j = 0; j < numEntries; j++) {
					gettimeofday(&curr_time, NULL);
					diff_t = difftime(curr_time.tv_sec, TS.topics[i].buffer[j].timeStamp.tv_sec);
					if (diff_t > delta) {
						pthread_mutex_lock(&mylock[th_idx]);
						dequeue(&myEntry, &TS.topics[i]);
						pthread_mutex_unlock(&mylock[th_idx]);
					}
					// If we hit an entry that's not old enough, all entries after that aren't either
					else {
						break; 
					}
				}
			}
		}
		else {
			sched_yield();
		}
	}
	return NULL;
}

int createPubs(threadPool *pubPool) {
	for (int i = 0; i < NUMPROXIES; i++) {
			pthread_create(&pubPool->threads[i], NULL, &publisher, (void *) &pubPool);
	}
	return 1;
}

int createSubs(threadPool *subPool) {
	for (int i = 0; i < NUMPROXIES; i++) {
		pthread_create(&subPool->threads[i], NULL, &publisher, (void *) &subPool);
	}
	return 1;
}

int joinPool(threadPool *pool) {
	for (int i = 0; i < NUMPROXIES; i++) {
		pthread_join(pool->threads[i], NULL);
	}
	return 1;
}

int destroyLock() {
	for (int i = 0; i < NUMPROXIES; i++) {
		pthread_mutex_destroy(&mylock[i]);
	}
	return 1;
}

int main(int argc, char const *argv[])
{
	// read file / getline variables
	FILE *fp;
	char *line;
	size_t len;
	char *token;
	int arg_ctr = 0;
	char *arg_arr[128];

	// open file
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "File <%s> open failure.\n", argv[1]);
        exit(EXIT_FAILURE);
	}

	// Topic variables
	int topicID;
	char topicName[NAMESIZE];
	int queueLen;

	TS.numTopics = 0;

	threadPool *pubPool, *subPool;
	cleanPool *clnPool;
	initPool(pubPool);
    initPool(subPool);
    // initLock();

	while (getline(&line, &len, fp) != EOF) {
		// clean up arg_arr in each iteration 
		for (int i = 0; i < arg_ctr; i++) {
			arg_arr[i] = NULL;
		}
		arg_ctr = 0;
		while ((token = strtok_r(line, " \"\r\n", &line))) {
			arg_arr[arg_ctr] = token;
			arg_ctr++;
		}

		if (strcmp(arg_arr[0], "create") == 0 && strcmp(arg_arr[1], "topic") == 0) {
			sscanf(arg_arr[2], "%d", &topicID);
			sscanf(arg_arr[4], "%d", &queueLen);
			// build queue
			initQueue(topicID, arg_arr[3], queueLen, &TS.topics[TS.numTopics]);
			TS.numTopics++;
			printf("Topic <%s> created - topicID: <%d> - length: <%d>\n", arg_arr[3], topicID, queueLen);
		}
		else if (strcmp(arg_arr[0], "query") == 0) {
			if (strcmp(arg_arr[1], "topics") == 0) {
				printf("query topic ...\n");
				for (int i = 0; i < TS.numTopics; i++) {
					printf("topicID: %d - topicName: %s - length: %d\n", 
						TS.topics[i].qid,
						TS.topics[i].name,
						TS.topics[i].length);
				}
			}
			else if (strcmp(arg_arr[1], "publishers") == 0) {
				printf("query publishers ...\n");
				for (int i = 0; i < pubPool->numFiles; i++) {
					printf("publisher: <%d> - command file: <%s>\n", i, pubPool->filename[i]);
				}
			}
			else if (strcmp(arg_arr[1], "subscribers") == 0) {
				printf("query subscribers ...\n");
				for (int i = 0; i < pubPool->numFiles; i++) {
					printf("subscriber: <%d> - command file: <%s>\n", i, subPool->filename[i]);
				}
			}
			else {
				fprintf(stderr, "Error! Can't query %s\n", arg_arr[1]);
				// exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg_arr[0], "add") == 0) {
			if (strcmp(arg_arr[1], "publisher") == 0 && pubPool->numFiles <= MAXPUBS) {
				strcpy(pubPool->filename[pubPool->numFiles], arg_arr[2]);
				pubPool->numFiles++;
				printf("Publisher <%d> to be read from <%s> added!\n", pubPool->numFiles, arg_arr[2]);
			}
			else if (strcmp(arg_arr[1], "subscriber") == 0 && subPool->numFiles <= MAXSUBS) {
				strcpy(subPool->filename[subPool->numFiles], arg_arr[2]);
				subPool->numFiles++;
				printf("Subscriber <%d> to be read from <%s> added!\n", subPool->numFiles, arg_arr[2]);
			}
			else {
				fprintf(stderr, "Error! Can't add %s\n", arg_arr[1]);
				// exit(EXIT_FAILURE);
			}
		}
		else if (strcpy(arg_arr[0], "delta") == 0) {
			sscanf(arg_arr[1], "%d", &clnPool->delta);
		}
		else if (strcpy(arg_arr[0], "start") == 0) {
			fclose(fp);
			break;
		}
		else {
			fprintf(stderr, "Error! Unsupport arguments\n");
			// exit(EXIT_FAILURE);
		}		
	}
	pthread_create(&clnPool->thread, NULL, &clean, (void *) &clnPool);
	createPubs(pubPool);
	createSubs(subPool);

	joinPool(pubPool);
	joinPool(subPool);

	destroyLock(mylock);
	
	return 0;
}














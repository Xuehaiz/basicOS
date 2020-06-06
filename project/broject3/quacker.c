/*=============================================================================
 * Program Name: quacker.c
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
	myPool->thread_idx = 0;
	myPool->isFree = 1;
	return 1;
}

void *publisher(void *voidPool) {
	threadPool *pubPool = (threadPool *) voidPool;

	pubPool->isFree = 0;
	pubParse(pubPool->filename);
	pubPool->isFree = 1;
	return NULL;
}

int pubParse(char *filename) {
	FILE *fpub;
	size_t len = 256;
	char *line = (char *)malloc(len * sizeof(char));
	char *saveptr;
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
		token = strtok_r(line, " \"\r\n", &saveptr);
		while (token != NULL) {
			arg_arr[arg_ctr] = token;
			token = strtok_r(NULL, " \"\r\n", &saveptr);
			arg_ctr++;
		}
		if (strcmp(arg_arr[0], "stop") == 0) {
			break;
		}
		else if (strcmp(arg_arr[0], "sleep") == 0) {
			sscanf(arg_arr[1], "%d", &sleep_t);
			usleep(sleep_t * 1000);
		}
		else if (strcmp(arg_arr[0], "put") == 0) {
			sscanf(arg_arr[1], "%d", &topicID);
			// find the topics' index 
			for (index = 0; index < TS.numTopics; index++) {
				if (TS.topics[index].qid == topicID) {
					break;
				}
			}
			// if the topic index cannot find, report an error
			if (index == TS.numTopics) {
				fprintf(stderr, "Error! Publisher <%ld> cannot find topic with ID <%d>\n", pthread_self(), topicID);
				fclose(fpub);
				return 0;
			}
			myEntry = initEntry(pthread_self(), arg_arr[2], arg_arr[3]);
			// Try enqueue for 30 times, and sleep 100 miliseconds in each failure attempt interval
			for (int i = 0; i < 30; i++) {
				success = enqueue(&myEntry, &TS.topics[index]);
				if (success) {
					printf("Publisher <%ld> enqueued a new entry to topic ID: <%d> with filename <%s>\n", pthread_self(), TS.topics[index].qid, TS.topics[index].name);
					break;
				}
				usleep(100000);
			}
			if (!success) {
				fprintf(stderr, "Error! Publisher <%ld> failed to enqueue a new entry to topic ID: <%d>\n", pthread_self(), topicID);
			}
		}
		else {
			fprintf(stderr, "Error! Publisher command does not recognize\n");
			// exit(EXIT_FAILURE);
		}
		sched_yield();
	}
	free(line);
	fclose(fpub);
	return 1;
}

void *subscriber(void *voidPool) {
	threadPool *subPool = (threadPool *) voidPool;

	subPool->isFree = 0;
	subParse(subPool->filename);
	subPool->isFree = 1;
	return NULL;
}

int subParse(char *filename) {
	FILE *fsub;
	size_t len = 256;
	char *line = (char *)malloc(len * sizeof(char));
	char *token;
	char *saveptr;
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

	// HTML handling
	char htmlfile[FILENAME_MAX];
	char *fname_cp = (char *)malloc(strlen(filename) + 1);
	strcpy(fname_cp, filename);
	token = strtok(fname_cp, ".");
	printf("fsub: %s\n", filename);
	strcpy(htmlfile, token);
	strcat(htmlfile, ".html");

	FILE *htmlptr = fopen(htmlfile, "w+");
	initHTML(htmlptr, filename);

	int lastEntry[TS.numTopics];
	for (int i = 0; i < TS.numTopics; i++) {
		lastEntry[i] = 0;
	}
	while (getline(&line, &len, fsub) != EOF) {
		for (int i = 0; i < arg_ctr; i++) {
			arg_arr[i] = NULL;
		}
		arg_ctr = 0;
		token = strtok_r(line, " \"\r\n", &saveptr);
		while (token != NULL) {
			arg_arr[arg_ctr] = token;
			token = strtok_r(NULL, " \"\r\n", &saveptr);
			arg_ctr++;
		}
		if (strcmp(arg_arr[0], "stop") == 0) {
			fclose(fsub);
			break;
		}
		else if (strcmp(arg_arr[0], "sleep") == 0) {
			sscanf(arg_arr[1], "%d", &sleep_t);
			usleep(sleep_t * 1000);
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
			// Try getEntry for 30 times, and sleep 100 milisecond in each failure attempt interval
			for (int i = 0; i < 30; i++) {
				entryNum = getEntry(lastEntry[index], &TS.topics[index], &myEntry);
				if (entryNum == 1) {
					printf("Subscriber <%ld> got entry <%d> from topic ID <%d>\n", pthread_self(), myEntry.entryNum, topicID);
					printf("URL: <%s> Caption: <%s>\n", myEntry.photoURL, myEntry.photoCaption);
					addHTML(htmlptr, TS.topics[index].name, myEntry.photoCaption, myEntry.photoURL);
					lastEntry[index]++;
					break;
				}
				else if (entryNum > 1) {
					printf("Subscriber <%ld> got entry <%d> from topic ID <%d>\n", pthread_self(), myEntry.entryNum, topicID);
					printf("URL: <%s> Caption: <%s>\n", myEntry.photoURL, myEntry.photoCaption);
					lastEntry[index] = entryNum;
					break;
				}
				usleep(100000);
			}
			if (!entryNum) {
				fprintf(stderr, "Error! Subscriber <%ld> failed to get entry from topic ID: <%d>\n", pthread_self(), topicID);
			}
		}
		else {
			fprintf(stderr, "Error! Subscriber command does not recognize\n");
			// exit(EXIT_FAILURE);
		}
		sched_yield();
	}
	free(line);
	free(fname_cp);
	endHTML(htmlptr);
	return 1;
}

void *clean(void *voidDelta) {
	useconds_t *delta = (useconds_t *)voidDelta;
	struct timeval curr_time;
	struct timeval lastclean;
	double diff_t;
	gettimeofday(&lastclean, NULL);
	int numEntries = 0;
	topicEntry myEntry;
	while (condition) {
		gettimeofday(&curr_time, NULL);
		diff_t = difftime(curr_time.tv_sec, lastclean.tv_sec);
		if (diff_t > *delta) {
			gettimeofday(&lastclean, NULL);
			for (int i = 0; i < TS.numTopics; i++) {
				numEntries = TS.topics[i].tail - TS.topics[i].head + 1;
				for (int j = 0; j < numEntries; j++) {
					gettimeofday(&curr_time, NULL);
					diff_t = difftime(curr_time.tv_sec, TS.topics[i].buffer[j].timeStamp.tv_sec);
					if (diff_t > *delta) {
						if (dequeue(&myEntry, &TS.topics[i])) {
							printf("Clean thread <%ld> dequeued entry <%d> from topic ID <%d> with filename <%s>\n", pthread_self(), myEntry.entryNum, TS.topics[i].qid, TS.topics[i].name);
							break;
						}
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

int destroyLock() {
	for (int i = 0; i < TS.numTopics; i++) {
		pthread_mutex_destroy(&TS.topics[i].mylock);
	}
	return 1;
}

void initHTML(FILE *htmlptr, char *fname) {
    fputs("<!DOCTYPE html>\n<html>\n<head>\n<title>HTML_SUBSCRIBER_FILENAME</title>\n\n", htmlptr);
    fputs("<style>\ntable, th, td {\n  border: 1px solid black;\n  ", htmlptr);
    fputs("border-collapse: collapse;\n}\nth, td {\n  padding: 5px;\n}\n", htmlptr);
    fputs("th {\n  text-align: left;\n}\n</style>\n\n</head>\n<body>\n\n", htmlptr);
    fputs("<h1>Subscriber: ", htmlptr);
    fputs(fname, htmlptr);
    fputs(" </h1>\n", htmlptr); 
    return NULL;
}

void addHTML(FILE *htmlptr, char *topicName, char* caption, char *url) {
	fputs("\n<h2>Topic Name: ", htmlptr);
    fputs(topicName, htmlptr);
    fputs("</h2>\n\n",htmlptr) 
	fputs("<table style=\"width:100%\" align=\"middle\">\n", htmlptr);
    fputs("  <tr>\n\t<th>CAPTION</th>\n\t<th>PHOTO-URL</th>\n  </tr>\n", htmlptr);
    fputs("  <tr>\n\t<td>", htmlptr);
    fputs(caption, htmlptr);
    fputs("</td>\n\t<td>", htmlptr);
    fputs(url, htmlptr);
    fputs("</td>\n  </tr>\n", htmlptr);
    fputs("</table>\n\n", htmlptr);
    return NULL;
}

void endHTML(FILE *htmlptr) {
	fputs("</body>\n</html>\n", htmlptr);
	return NULL;
}

int main(int argc __attribute__((unused)), char const *argv[])
{
	// read file / getline variables
	FILE *fp;
	char *saveptr;
	size_t len = 256;
	char *line = (char *)malloc(len * sizeof(char));
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
	int iter = 0;
	int pub_idx = 0;
	int sub_idx = 0;
	int pubFileCtr = 0;
	int subFileCtr = 0;
	char pub_file_arr[MAXPUBS][FILENAME_MAX];
	char sub_file_arr[MAXSUBS][FILENAME_MAX];
	int topicID;
	int queueLen;
	int delta_t;

	TS.numTopics = 0;
	
	threadPool pubPool[NUMPROXIES];
	threadPool subPool[NUMPROXIES];

	for (int i = 0; i < NUMPROXIES; i++) {
		initPool(&pubPool[i]);
		initPool(&subPool[i]);
	}

	while (getline(&line, &len, fp) != EOF) {
		// clean up arg_arr in each iteration 
		for (int i = 0; i < arg_ctr; i++) {
			arg_arr[i] = NULL;
		}
		arg_ctr = 0;
		token = strtok_r(line, " \"\r\n", &saveptr);
		while (token != NULL) {
			arg_arr[arg_ctr] = token;
			token = strtok_r(NULL, " \"\r\n", &saveptr);
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
				for (int i = 0; i < pubFileCtr; i++) {
					printf("publisher: <%d> - command file: <%s>\n", i+1, pub_file_arr[i]);
				}
			}
			else if (strcmp(arg_arr[1], "subscribers") == 0) {
				printf("query subscribers ...\n");
				for (int i = 0; i < subFileCtr; i++) {
					printf("subscriber: <%d> - command file: <%s>\n", i+1, sub_file_arr[i]);
				}
			}
			else {
				fprintf(stderr, "Error! Can't query %s\n", arg_arr[1]);
				// exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg_arr[0], "add") == 0) {
			if (strcmp(arg_arr[1], "publisher") == 0 && pubFileCtr <= MAXPUBS) {
				strcpy(pub_file_arr[pubFileCtr], arg_arr[2]);
				pubFileCtr++;
				// find a free thread and create thread
				iter = 0;
				while (TRUE) {
					if (iter >= NUMPROXIES) {
						iter = 0;
					}
					if (pubPool[iter].isFree == 1) {
						pub_idx = iter;
						break;
					}

					iter++;
					usleep(100000);
				}
				strcpy(pubPool[pub_idx].filename, arg_arr[2]);
				pubPool[pub_idx].thread_idx = pub_idx;
				pthread_create(&pubPool[pub_idx].thread, NULL, &publisher, (void *) &pubPool[pub_idx]);
				printf("Publisher <%d> to be read from <%s> added!\n", pubFileCtr, arg_arr[2]);
			}
			else if (strcmp(arg_arr[1], "subscriber") == 0 && subFileCtr <= MAXSUBS) {
				strcpy(sub_file_arr[subFileCtr], arg_arr[2]);
				subFileCtr++;
				iter = 0;
				while (TRUE) {
					if (iter >= NUMPROXIES) {
						iter = 0;
					}
					if (subPool[iter].isFree == 1) {
						sub_idx = iter;
						break;
					}
					iter++;
					usleep(100000);
				}
				strcpy(subPool[sub_idx].filename, arg_arr[2]);
				subPool[pub_idx].thread_idx = sub_idx;
				pthread_create(&subPool[sub_idx].thread, NULL, &subscriber, (void *) &subPool[sub_idx]);
				printf("Subscriber <%d> to be read from <%s> added!\n", subFileCtr, arg_arr[2]);
			}
			else {
				fprintf(stderr, "Error! Can't add %s\n", arg_arr[1]);
				// exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg_arr[0], "delta") == 0) {
			sscanf(arg_arr[1], "%d", &delta_t);
		}
		else if (strcmp(arg_arr[0], "start") == 0) {
			printf("Program starting...\n");
			// fclose(fp);
			break;
		}
		else {
			fprintf(stderr, "Error! Unsupport arguments\n");
			// exit(EXIT_FAILURE);
		}
		printf("---------------------------------------------------------------\n");
	}
	fclose(fp);

	pthread_t cleanThread;
	pthread_create(&cleanThread, NULL, &clean, (void *) &delta_t);

	for (int i = 0; i < pubFileCtr; i++) {
		if (pthread_join(pubPool[i].thread, NULL)) {
			fprintf(stderr, "Error! Thread pubPool[%d] join failed\n", i);
		}
	}
	
	for (int i = 0; i < subFileCtr; i++) {
		if (pthread_join(subPool[i].thread, NULL)) {
			fprintf(stderr, "Error! Thread subPool[%d] join failed\n", i);
		}
	}

	sleep(40);
	condition = 0;

	if (pthread_join(cleanThread, NULL)) {
		fprintf(stderr, "Error! Clean thread join failed\n");
	}

	free(line);
	destroyLock();
	
	return 0;
}
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "InstaQuack.h"

void initialize() { // int qID
	for (int i = 0; i < MAXQUEUES; i++) {
		queues[i].head = 0;
		queues[i].tail = 0;
		queues[i].counter = 0;
		queues[i].length = MAXENTRIES;
		//queues[i].qID = qID;
	}
}


int enqueue(int TQ_ID, struct topicEntry TE) {
	for (int i = 0; i < MAXQUEUES; i++) {
		if (queues[i].qID == TQ_ID) {
			if ((queues[i].tail - queues[i].head) == -1) { // buffer is full
				return 0;
			}
			if (queues[i].buffer[queues[i].tail].entryNum != -1) {
				TE.entryNum = queues[i].counter;
				queues[i].counter++;
				queues[i].buffer[queues[i].tail] = TE;
				gettimeofday(&(queues[i].buffer[queues[i].tail].timeStamp), NULL);
				if (queues[i].tail == queues[i].length) {
					queues[i].tail = 0;
				}
				else {
					queues[i].tail++;
				}
				return 1;
			}
		}
	}
	return 0;
}


int dequeue(int TQ_ID, struct topicEntry *TE) {
	for (int i = 0; i < MAXQUEUES; i++) {
		if (queues[i].qID == TQ_ID) {
			if (queues[i].head == queues[i].tail) {  // buffer is empty
				return 0;
			}
			TE->entryNum = queues[i].buffer[queues[i].head].entryNum;
			TE->timeStamp = queues[i].buffer[queues[i].head].timeStamp;
			TE->pubID = queues[i].buffer[queues[i].head].pubID;
			strcpy(TE->photoURL, queues[i].buffer[queues[i].head].photoURL);
			strcpy(TE->photoCaption, queues[i].buffer[queues[i].head].photoCaption);

			// change the null entry to empty, might be unnecessary 
			if (queues[i].head == 0) {
				queues[i].buffer[queues[i].length].entryNum = 0;
			}
			else {
				queues[i].buffer[queues[i].head - 1].entryNum = 0;
			}
			// change the current entry to null
			queues[i].buffer[queues[i].head].entryNum = -1;

			// increment the head
			if (queues[i].head == queues[i].length) {
				queues[i].head = 0;
			}
			else {
				queues[i].head++;
			}
			return 1;
		}
	}
	return 0;
}


int getEntry(int TQ_ID, int lastEntry, struct topicEntry *TE) {
	int found = 0;
	int index;
	for (int i = 0; i < MAXQUEUES; i++) {
		if (queues[i].qID == TQ_ID) {
			// Case1: when queue is empty 
			if (queues[i].head == queues[i].tail) {
				return 0;
			}
			else {
				for (int j = 0; j < MAXENTRIES; j++) {
					index = (queues[i].head + j) % queues[i].length;
					if (queues[i].buffer[index].entryNum == lastEntry + 1) {
						found = 1;
						break;
					}
				}
				// Case 2: next entry is found in the topicQueue
				if (found) {
					TE->entryNum = queues[i].buffer[index].entryNum;
					TE->timeStamp = queues[i].buffer[index].timeStamp;
					TE->pubID = queues[i].buffer[index].pubID;
					strcpy(TE->photoURL, queues[i].buffer[index].photoURL);
					strcpy(TE->photoCaption, queues[i].buffer[index].photoCaption);
					return 1;
				}
				// Case 3: topicQueue is not empty but next entry is not found
				else {
					for (int k = 0; k < MAXENTRIES; k++) {
						index = (queues[i].head + k) % queues[i].length;
						if (queues[i].buffer[index].entryNum > lastEntry + 1) {
							found = 1;
							break;
						}
					}
					// Case 3-ii: The lastEntry + 1 was dequeued by the cleanup thread. 
					// The calling thread should update its lastEntry to the entryNum.
					if (found) {
						TE->entryNum = queues[i].buffer[index].entryNum;
						TE->timeStamp = queues[i].buffer[index].timeStamp;
						TE->pubID = queues[i].buffer[index].pubID;
						strcpy(TE->photoURL, queues[i].buffer[index].photoURL);
						strcpy(TE->photoCaption, queues[i].buffer[index].photoCaption);
						return queues[i].buffer[index].entryNum;
					}
					// Case 3-i: desired entry is yet to arrive
					else {
						return 0;
					}
				}
			}
		}
	}
	return 0;
}


void *publisher(void *args) {
	char *pubfile = ((struct Threads *) args)->filename;
	FILE *fp;
	fp = fopen(pubfile, "r");
	if (fp == NULL) {
		fprintf(stderr, "publisher file open failure. \n");
	}
	char line[128];
	char *token;
	char *arg_arr[128];
	int qID;
	useconds_t sleep_t;
	// int count = 0; 
	int counter = 0;
	while (fgets(line, 128, fp) != NULL) {
		for (int i = 0; i < counter; i++) {
			arg_arr[i] = NULL;
		}
		counter = 0;
		token = strtok(line, " \"\r\n");
		while (token != NULL) {
			arg_arr[counter] = token;
			token = strtok(NULL, " \"\r\n");
			counter++;
		}
		if (strcmp(arg_arr[0], "stop") == 0) {
			fclose(fp);
			((struct Threads *) args)->alive = 1;
			break;
		}
		else if (strcmp(arg_arr[0], "sleep") == 0) {
			sscanf(arg_arr[1], "%d", &sleep_t);
			usleep(sleep_t);
		}
		else if (strcmp(arg_arr[0], "put") == 0) {
			sscanf(arg_arr[1], "%d", &qID);
			struct topicEntry TE;
			strcpy(TE.photoURL, arg_arr[2]);
			strcpy(TE.photoCaption, arg_arr[3]);
			pthread_mutex_lock(&(mutex[qID]));
			
			while (!enqueue(qID, TE)) {
				fprintf(stderr, "publisher <%ld> - enqueue entry failure\n", pthread_self());
				usleep(500);
			}
			pthread_mutex_unlock(&(mutex[qID]));
			printf("publisher <%ld> - enqueue entry success\n", pthread_self());
			/*else {
				pthread_mutex_unlock(&(mutex[qID]));
				fprintf(stderr, "publisher <%d> - enqueue entry failure\n", qID);
			}*/
		}
		else {
			fprintf(stderr, "Error! Publisher command does not recognize\n");
			exit(EXIT_FAILURE);
		}
		sched_yield();
	}
	((struct Threads *) args)->alive = 1;
	return NULL;
}


void *subscriber(void *args) {
	char *subfile = ((struct Threads *) args)->filename;
	FILE *fp;
	fp = fopen(subfile, "r");
	if (fp == NULL) {
		fprintf(stderr, "subscriber file open failure. \n");
	}
	char line[128];
	char *token;
	char *arg_arr[128];
	int qID;
	useconds_t sleep_t;
	int counter = 0;
	int count = 0;
	struct topicEntry TE;
	int lastEntry[MAXQUEUES];
	for (int i = 0; i < MAXQUEUES; i++) {
		lastEntry[i] = 0;
	}

	while (fgets(line, 128, fp) != NULL) {
		for (int i = 0; i < counter; i++) {
			arg_arr[i] = NULL;
		}
		counter = 0;
		token = strtok(line, " \r\n");
		while (token != NULL) {
			arg_arr[counter] = token;
			token = strtok(NULL, " \r\n");
			counter++;
		}
		if (strcmp(arg_arr[0], "stop") == 0) {
			fclose(fp);
			((struct Threads *) args)->alive = 1;
			break;
		}
		else if (strcmp(arg_arr[0], "sleep") == 0) {
			sscanf(arg_arr[1], "%d", &sleep_t);
			usleep(sleep_t);
		}
		else if (strcmp(arg_arr[0], "get") == 0) {
			sscanf(arg_arr[1], "%d", &qID);
			pthread_mutex_lock(&(mutex[qID]));
			int entry = getEntry(qID, lastEntry[qID], &TE);
			pthread_mutex_unlock(&(mutex[qID]));
			if (entry == 1) {
				printf("subscriber <%ld> - get entry <%d> success\n", pthread_self(), TE.entryNum);
				printf("Entry number <%d> - URL <%s> - Caption <%s>\n", TE.entryNum, TE.photoURL, TE.photoCaption);
				lastEntry[qID]++; 
			}
			else if (entry > 1) {
				printf("subscriber <%ld> - get entry <%d> success\n", pthread_self(), TE.entryNum);
				printf("Entry number <%d> - URL <%s> - Caption <%s>\n", TE.entryNum, TE.photoURL, TE.photoCaption);
				lastEntry[qID] = entry;
			}
			else {
				fprintf(stdout, "subscriber <%ld> - get entry failure\n", pthread_self());
				// sleep 500 ms if get entry failed
				usleep(500);
			}
		}
		else {
			fprintf(stderr, "Error! Subscriber command does not recognize\n");
			exit(EXIT_FAILURE);
		}
		sched_yield();
	}
	((struct Threads *) args)->alive = 1;
	return NULL;
}


void *cleanup() {  // void *delta
	// time_t delta_t = (time_t) delta;
	struct timeval curr_time;
	struct timeval lastclean;
	double diff_t;
	struct topicEntry TE;
	gettimeofday(&lastclean, NULL);
	while (condition) {
		gettimeofday(&curr_time, NULL);
		diff_t = difftime(curr_time.tv_sec, lastclean.tv_sec);
		if (diff_t > queues[0].age) {
			gettimeofday(&lastclean, NULL);
			for (int i = 0; i < MAXQUEUES; i++) {
				for (int j = 0; j < MAXENTRIES; j++) {
					gettimeofday(&curr_time, NULL);
					diff_t = difftime(curr_time.tv_sec, queues[i].buffer[j].timeStamp.tv_sec);
					if (diff_t > queues[i].age) {  // (double) delta_t
						pthread_mutex_lock(&mutex[i]);
						if (dequeue(queues[i].qID, &TE)) {
							printf("Cleanup thread <%ld> dequeued entry <%d>\n", pthread_self(), queues[i].buffer[j].entryNum);
						}  
						pthread_mutex_unlock(&mutex[i]);
					}
					else {
						// If we hit an entry that's not old enough, all entries after that aren't either
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


int main(int argc, char const *argv[])
{
	int numtopics = 0;

	int counter = 0;
	char line[128];
	char *token;
	char *arg_arr[128];
	int argv_1;

	int qID; 
	int pub_idx = 0;
	int sub_idx = 0;
	int pub_count = 0;
	int sub_count = 0;

	// initialize pools 
	for (int i = 0; i < NUMPROXIES; i++) {
		pubs[i].alive = 1;
		subs[i].alive = 1;
	}

	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "File <%s> open failure.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	// create clean-up thread 
	pthread_attr_init(&attr);
	pthread_create(&cleanThread, &attr, cleanup, NULL);  // (void *) &delta_t
	
	while (fgets(line, 128, fp) != NULL) {
		// clean up arg_arr in each iteration 
		for (int i = 0; i < counter; i++) {
			arg_arr[i] = NULL;
		}
		counter = 0;
		token = strtok(line, " \"\r\n");
		while (token != NULL) {
			arg_arr[counter] = token;
			token = strtok(NULL, " \"\r\n");
			counter++;
		}

		/*for (int i = 0; i < counter; i++) {
			printf("arg_arr[%d]: <%s>;   ", i, arg_arr[i]);
		}
		// break;
		printf("\n");*/

		if (strcmp(arg_arr[0], "create") == 0 && strcmp(arg_arr[1], "topic") == 0) {
			sscanf(arg_arr[2], "%d", &qID);
			queues[numtopics].qID = qID;
			strcpy(queues[numtopics].name, arg_arr[3]);
			queues[numtopics].head = 0;
			queues[numtopics].tail = 0;
			queues[numtopics].length = MAXENTRIES;
			queues[numtopics].counter = 0;
			numtopics++;
		}
		else if (strcmp(arg_arr[0], "query") == 0) {
			// printf("strcmp: %d\n", strcmp("topics", arg_arr[1]));
			if (strcmp(arg_arr[1], "topics") == 0) {
				printf("query topic ...\n");
				for (int i = 0; i < numtopics; i++) {
					printf("Topic name <%s>, ID: <%d>, length: <%d>\n",
						queues[i].name, 
						queues[i].qID, 
						queues[i].length);
				}
			}
			else if (strcmp(arg_arr[1], "publishers") == 0) {
				printf("query publishers ...\n");
				printf("current publisher: <%d> - command file: <%s>\n", 
					pub_count, 
					pubs[pub_idx].filename);
			}
			else if (strcmp(arg_arr[1], "subscribers") == 0) {
				printf("query subscribers ...\n");
				printf("current subscriber: <%d> - command file: <%s>\n",
					sub_count,
					subs[sub_idx].filename);
			} 
			else {
				fprintf(stderr, "Error! Can't query %s\n", arg_arr[1]);
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg_arr[0], "add") == 0) {
			if (strcmp(arg_arr[1], "publisher") == 0) {
				// find a free thread 
				int iter = 0;
				while (TRUE) {
					if (iter >= MAXPUBS) {
						iter = 0;
					}
					if (pubs[iter].alive == 1) {
						pub_idx = iter;
						break;
					}
					iter++;
				}
				pubs[pub_idx].tid = pub_idx;
				pubs[pub_idx].alive = 0;
				pubs[pub_idx].accumulator = pub_count;
				strcpy(pubs[pub_idx].filename, arg_arr[2]);
				pthread_attr_init(&attr);
				pthread_create(&pubs[pub_idx].th, &attr, publisher, (void *) &pubs[pub_idx]);
				pub_count++;
			}
			else if (strcmp(arg_arr[1], "subscriber") == 0) {
				int iter = 0;
				while (TRUE) {
					if (iter >= MAXSUBS) {
						iter = 0;
					}
					if (subs[iter].alive == 1) {
						sub_idx = iter;
						break;
					}
					iter++;
				}
				subs[sub_idx].tid = sub_idx;
				subs[sub_idx].alive = 0;
				subs[sub_idx].accumulator = sub_count;
				strcpy(subs[sub_idx].filename, arg_arr[2]);
				pthread_attr_init(&attr);
				pthread_create(&subs[sub_idx].th, &attr, subscriber, (void *) &subs[sub_idx]);
				sub_count++;
			}
			else {
				fprintf(stderr, "Error! Can't add %s\n", arg_arr[1]);
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg_arr[0], "delta") == 0) {
			sscanf(argv[1], "%d", &argv_1);
			time_t delta_t = argv_1;
			for (int i = 0; i < MAXQUEUES; i++) {
				queues[i].age = delta_t;
			}
		}
		else if (strcmp(arg_arr[0], "start") == 0) {
			fclose(fp);
			break;
		}
		else {
			fprintf(stderr, "Error! Unsupport arguments\n");
			exit(EXIT_FAILURE);
		}
		// printf("here\n");
	}

  	// join
  	for (int i = 0; i < MAXPUBS; i++) {
  		if (pthread_join(pubs[i].th, NULL) != 0) {
  			fprintf(stderr, "pubs[%d] thread join failure\n", i);
  			// exit(EXIT_FAILURE);
  		}
  		else {
  			printf("pubs[%d] thread join succeed\n", i);
  		}
  	}

  	for (int i = 0; i < MAXSUBS; i++) {
  		if (pthread_join(subs[i].th, NULL) != 0) {
  			fprintf(stderr, "subs[%d] thread join failure\n", i);
  			// exit(EXIT_FAILURE);
  		}
  		else {
  			printf("subs[%d] thread join succeed\n", i);
  		}
  	}

  	condition = 0;

  	pthread_join(cleanThread, NULL);

  	for (int i = 0; i < MAXQUEUES; i++) {
  		pthread_mutex_destroy(&(mutex[i]));
  	}

	return 0;
}


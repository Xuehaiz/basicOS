/*
* Description: Project 3
*
* Author: Missy Shi
*
* Course: CIS 415
*
* Date: 05/15/2020
*
* Notes:
* 1. Implement a circular ring buffer capable of holding ​MAXENTRIES​ topic entries,
* 	 where each topic entry consists of the struct
* 	 There will be MAXTOPICS total topic queues
* 
* Credits:
* 	https://www.tutorialspoint.com/cprogramming/c_structures.htm
*	https://stackoverflow.com/questions/13706809/structs-in-c-with-initial-values
*	https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
*
* 	Donna Hooshmand
*		explained to me how the topic entries and queues work
*		how to use struct, how timestamp works
* 
*	http://man7.org/linux/man-pages/man3/memcpy.3.html
*/

/*-------------------------Preprocessor Directives---------------------------*/
#define _GNU_SOURCE
// #define _POSIX_C_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include "topicQueue.h"
/*---------------------------------------------------------------------------*/

topicQueue *registry[MAXTOPICS]; // Store all topics in here
threads pubs[NUMPROXIES];	// thread pool for publisher
threads subs[NUMPROXIES];	// thread pool for subscriber
pthread_attr_t attr;		// thread attributes
pthread_mutex_t lock[MAXTOPICS]; // mutex lock for queues

int inserted = 1; // # of entries inserted

void initialize() {
	int i;
	// initialize each queue's information
	for (i = 0; i < MAXTOPICS; i++) {
		registry[i]->head = 0;
		registry[i]->tail = 0;
		registry[i]->length = NUMENTRIES;
	}
	for (i = 0; i < MAXTOPICS; i++) {
		phtread_mutex_init(&(mutex[i]), NULL);
	}
	pthread_attr_init(&attr);
	for (i = 0; i < NUMPROXIES; i++) {
		pubs[i].alive = 0;
		subs[i].alive = 0;
	}
} // initialize()



int enqueue(char *QID, struct topicEntry *TE) // TE is a topic entry to be pushed to the queue
{
	/*
	int enqueue(q => QID / int, TE) {
		// enqueue TE into queue
	}
	*/
	struct timeval now;
	for (int i = 0; i < MAXTOPICS; i++) {
		if (strcmp(QID, registry[i]->name) == 0) { // this Queue is in the registry
			// if (isFull(registry[i])) { printf("This topic queue is full\n"); return 0; } 
			int t = registry[i]->tail;
			if (registry[i]->entries[t].entryNum != -1) {
				// save entryNum
				registry[i]->entries[t].entryNum = inserted;	// insert a new entry, current entryNum -> number of total inserted
				// increment entry counter
				inserted++;
				// place new entry on at tail
				registry[i]->entries[t] = *TE;
				gettimeofday(&now, NULL);
				registry[i]->entries[t].timeStamp = now;
				
				// registry[i]->tail = (registry[i]->tail + 1) % MAXTOPICS;
				if (t == registry[i]->length) { registry[i]->tail = 0; }
				else { registry[i]->tail++; }
				return 1;
			}
		}
	}
	return 0;
} // enqueue(char *QID, struct topicEntry *TE)

// split dequeue function in lab8 into getEntry and dequeue

int getEntry(char *QID, int lastEntry, topicEntry *TE) // TE​ is an empty topicEntry​ struct
{
	/*
	similar to dequeue in lab8
	... get an entry and copy to TE
	*/

	for (int i = 0; i < MAXTOPICS; i++) {
		if (registry[i] != NULL) {
			if (strcmp(registry[i]->name, QID) == 0) {
				// CASE 1​: ​topic queue is empty​
				if (registry[i]->tail == registry[i]->head) { printf("getEntry: queue is empty\n"); return 0; }
				// CASE 2​: ​lastEntry+1​ ​entry is in the queue, return 1
				// scan the queue entries, starting with the oldest entry in the queue
				for (int j = 0; j < NUMENTRIES; j++) {
					// if it finds the lastEntry+1 entry
					if (lastEntry+1 == registry[i]->entries[j].entryNum) {
						// copies the entry into the empty ​topicEntry structure (TE)
						*TE = registry[i]->entries[j];
						printf("getEntry: found lastEntry+1\n");
						return 1;
					}
				}
				// CASE 3​: ​topic queue not empty && ​lastEntry+1​ entry is not the queue​
				// SUB-CASE 2:
				// scan the queue entries, starting with the oldest entry in the queue
				for (int j = 0; j < NUMENTRIES; j++) {
					// If it encounters an entry greater than lastEntry+1
					if (registry[i]->entries[j].entryNum > lastEntry+1) {
						// copy entry into TE and return the entryNum of that entry
						printf("getEntry: found an entry greater than lastEntry+1\n");
						return registry[i]->entries[j].entryNum;
					}
				}
				// SUB-CASE 1:
				// If all entries in the queue are less than lastEntry+1, 
				// that entry has yet to be put into the queue -> return 0
				printf("getEntry: all entries in the queue less than lastEntry+1\n");
				return 0;
			} 
		} 
	}
	printf("getEntry: QID invalid\n");
	return 0;
} // getEntry(char *QID, int lastEntry, topicEntry *TE)


int dequeue(char *QID)
{
	/* 
	CLEAN a specific q
	for one q, 
	iterate whole q, 
	clean everything up

	now = gettimeofday();
	loop through q:
		duration = now - TE.timestamp
		if duration > delta:
			pop
	*/
	// clean out the entries that are outdated
	long long duration;
	struct timeval prev;
	struct timeval now;

	for (int i = 0; i < NUMENTRIES; i++) {
		if (registry[i] != NULL) {
			if (strcmp(registry[i]->name, QID) == 0) {
				if (registry[i]->tail == registry[i]->head) { printf("dequeue: registry is empty\n"); return 0; }
				// If a topic entry ages ​DELTA beyond when it was inserted into the queue, it should be dequeued.
				gettimeofday(&now, NULL);
				prev = registry[i]->entries[registry[i]->head].timeStamp;
				duration = now.tv_sec - prev.tv_sec;
				duration = (duration * 1e6 + duration) * 1e6;
				if (duration < DELTA) {
					printf("dequeue: not rejected since it is not outdated\n");
					return 0;
				}

				// FIXME
				if (registry[i]->entries[registry[i]->tail].entryNum == -1) {
					registry[i]->entries[registry[i]->head].entryNum = -1;
					int h = (registry[i]->head - 1) % NUMENTRIES;
					if (h == -1) { h = NUMENTRIES; }
					registry[i]->entries[h].entryNum = 0;
					registry[i]->head = (registry[i]->head + 1) % NUMENTRIES;
				} else {
					int h = (registry[i]->head - 1) % NUMENTRIES;
					if (h == -1) { h = NUMENTRIES; }
					registry[i]->entries[h].entryNum = 0;
					registry[i]->entries[registry[i]->head].entryNum = -1;
					registry[i]->head = (registry[i]->head + 1) % NUMENTRIES;;
				}
				return 1;
			}
		}
	}
	printf("dequeue: invalid QID\n");
	return 0;
} // dequeue(char *QID)

void *cleanup()
{
	/* cleanup thread
	while threads alive: 
        until it's time to clean
			yield
		when time to clean:
			for each topic:
				dequeue
	
	// loop until every thread is dead
	gettimeofday(&lastClean, NULL);
	while (condition) {

		if find any thread alive:
		loop through subs:
			if subs[i].alive == 1:
				no_thread_alive = 0;
		...

		same for publisher
		
		gettimeofday(&now, NULL);	// now = current time
		doClean = now - lastClean
		if doClean > DELTA:
			gettimeofday(&lastClean, NULL); // lastClean = now
			loop through every topic:
				lock()
				dequeue(q, DELTA)
				unlock
		sched_yield(...);

		if (no_thread_alive) { condition = 0; }
	}
	*/
	int condition = 1;
	// int no_thread_alive;
	// int now;
	// int doClean, lastClean;
	// gettimeofday(&lastClean, NULL);
	// while (condition) {
	// 	no_thread_alive = 1;
	// 	for (int i = 0; i < NUMPROXIES; i++) {
	// 		if (subs[i].alive == 1) { no_thread_alive = 0; }
	// 	}
	// 	for (int i = 0; i < NUMPROXIES; i++) {
	// 		if (pubs[i].alive == 1) { no_thread_alive = 0; }
	// 	}
	// 	gettimeofday(&now, NULL);
	// 	doClean = now - lastClean;
	// 	if doClean > DELTA {
	// 		gettimeofday(&lastClean, NULL);
	// 		for (int i = 0; i < MAXTOPICS; i++) {
	// 			lock();
	// 			dequeue(DELTA, registry[i]);
	// 			unlock();
	// 		}
	// 	}

	// 	if (no_thread_alive) { condition = 0; }	// if no thread alive --> break the look
	// }

	while (condition) {
		for (int i = 0; i < MAXTOPICS; i++) {
			if (registry[i] != NULL) {
				pthread_mutex_lock(&lock[i]);
				while (dequeue(registry[i]->name)) { printf("cleaning some outdated entries\n"); }
			}	pthread_mutex_unlock(&lock[i]);
		}
		usleep(1);
	}
	return NULL;
} // cleanup()


void *publisher(void *fpub)
{
	/* publisher thread
	hardcode a put
	// Synchronization ways: 
		1. q as an object, store mutex in the q
		2. an array of locks / mutex in global (struct)
	lock(...)
	enqueue() => put some info into specific q
		e.g.: enqueue("Mountains", TE, ...)	=> specific info
	unlock(...)
	===> do not put lock in enqueue or dequeue
	*/
	int condition = 1;
	topicEntry TE;
	while (condition) {
		for (int i = 0; i < MAXTOPICS; i++) {
			if (registry[i] != NULL && strcmp(registry[i]->name, (char *)fpub) == 0) {
				// pthread_mutex_lock(&lock[i]);
				int result = enqueue(registry[i]->name, &TE);
				// pthread_mutex_unlock(&lock[i]);

				while (!result) {
					// pthread_mutex_lock(&lock[i]);
					result = enqueue(registry[i]->name, &TE);
					// pthread_mutex_unlock(&lock[i]);
					usleep(1);
					// sched_yield();
				}
				usleep(1);
			}
		}
	}
	// thread 有一个struct，如果跑完一轮，就把alive = 0
	return NULL;
} // publisher(void *fpub)


void *subsriber(void *fsub)
{
	/* subscriber thread
	usleep(2000)

	// hardcoding some gets
	lock()
	getEntry(...)
	unlock()

	lock()
	getEntry(...)
	unlock()

	lock()
	getEntry(...)
	unlock()
	
	subs[thisthread].alive = 0;
	return 1;
	*/
	topicEntry *TE;
	TE.entryNum = -1;
	int lastEntry = 1;
	int i;
	int condition = 1;
	while (condition) {
		for (i = 0; i < MAXTOPICS; i++) {
			if (registry[i] != NULL && strcmp(registry[i]->name, (char *)fsub) == 0) {
				// pthread_mutex_lock(&lock[i]);
				int result = getEntry(*registry[i]->name, lastEntry, &TE);
				if (result) { lastEntry++; }
				else if (!result) {
					usleep(1);
					sched_yield();
				}
				else {
					lastEntry = result;
				}
				usleep(1);
			}
		}
	}
	return NULL;

} // subscriber()




int main(int argc, char const *argv[])
{
	// create threads in main
	/*
	part 3:
	// read main.cfg
	loop through input files:
		if(create): add pub --> 
		if(query):
		...

		start command


	part 2:
	since already have the table and pools
	all part2 needs ==> 
	// init threads in main
	*/

	if (argc != 2) { fprintf(stderr, "Usage: %s <filename.txt>\n", argv[0]); return 0; }
	size_t bufsize = 256;
	FILE *fmain, fpub, fsub;
	char *buf = NULL;
	// char *cmd[8] = {"create topic", "query topics", "add publisher", "query publishers", "add subscriber", "query subscribers", "delta", "start"};
	char *cmd[bufsize], *token, *rest, *rem;
	fmain = fopen(argv[1], "r");
	if (fmain == NULL) { perror("fopen(mainfile)"); return 0; }

	while (getline(&buf, &bufsize, fmain) != EOF) {
		/* Possible commands in main:
           - create topic <topic ID> "<topic name>" <queue length>
           - query topics
           - add publisher "<publisher command file>"
           - query publishers
           - add subscriber "<subscriber command file>"
           - query subscribers
           - delta <DELTA>
           - start
        */
		rest = buf;
		while ((token = strtok_r(rest, "\n", &rest))) {
			count = 0;
			rem = token;
			if (strcmp(token, " ") == 0) { fprintf(stderr, "Error! Unrecognized command: %s\n",token); break;}
			char *param;
			while ((param = strtok_r(rem, " ", &rem))) {
				cmd[count] = param;
				count++;
			}
		}
	}
	

	return 0;
}







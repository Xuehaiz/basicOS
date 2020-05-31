#ifndef INSTAQUACK_H_
#define INSTAQUACK_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define MAXENTRIES 5
#define MAXQUEUES 3
#define NUMPROXIES 2
#define NAMESIZE 20
#define URLSIZE 30
#define CAPSIZE 30
#define MAXPUBS	2
#define MAXSUBS	2	
#define TRUE 1	

struct topicEntry {
	int entryNum;
	struct timeval timeStamp;
	int pubID;
	char photoURL[URLSIZE]; // URL to photo
	char photoCaption[CAPSIZE]; // photo caption
} topicEntry;

struct topicQueue {
	int qID;
	char name[NAMESIZE];
	struct topicEntry buffer[MAXENTRIES];
	int head, tail;
	int length;
	time_t age;
	int counter;
} topicQueue;

struct Threads {
	int alive;
	pthread_t th;
	char filename[32];
	int accumulator;
	int tid;
} Threads;

struct topicQueue queues[MAXQUEUES]; 

pthread_mutex_t mutex[MAXQUEUES] = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_sub = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_pub = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_clean = PTHREAD_MUTEX_INITIALIZER;

pthread_t cleanThread;  // for cleanup thread 
pthread_attr_t attr;

pthread_t publish;
pthread_t subscribe;

struct Threads pubs[MAXPUBS];
struct Threads subs[MAXPUBS];

int condition = 1;

void initialize();

int enqueue(int TQ_ID, struct topicEntry TE);

int dequeue(int TQ_ID, struct topicEntry *TE);

int isEmpty();

int getEntry(int TQ_ID, int lastEntry, struct topicEntry *TE);

void *publisher(void *pub_file);

void *subscriber(void *sub_file);

void *cleanup(void *delta);

#endif


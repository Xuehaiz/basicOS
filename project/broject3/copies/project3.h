/*
* Description: Project 3 - header file
*
* Author: Missy Shi
*
* Course: CIS 415
*
* Date: 05/22/2020
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

#ifndef PROJECT3_H_
#define PROJECT3_H_
/*--------------------------Macro Configurations---------------------------*/
#define TRUE 1
#define URLSIZE		8	// URL size
#define CAPSIZE		8	// caption size
// #define ​MAXENTRIES​  8	// total topic entries
#define NUMENTRIES  8
#define MAXTOPICS	8	// total topic queues
#define MAXPUBS		8	// maximum number of publishers
#define MAXSUBS		8	// maximum number of subscribers
#define MAXNAME		8	// max of name
#define NUMPROXIES	8
#define DELTA		4	// topic entry ages
/*---------------------------------------------------------------------------*/

/*-------------------------Structures Declaration---------------------------*/
typedef struct topicEntry {
	int entryNum;
	struct timeval timeStamp;
	int pubID;
	char photoURL[URLSIZE];		// URL to photo
	char photoCaption[CAPSIZE];	// photo caption
} topicEntry;

typedef struct topicQueue {
	int head;	// the newest entry in the topic queue
	int tail;	// the oldest entry in the topic queue
	int length;	// length of each entries
	char name[MAXNAME];	// topic name -> QID
	struct topicEntry entries[NUMENTRIES];
	// pthread_mutex_t lock;
} topicQueue;

typedef struct threads {
	pthread_t thread;
	int alive;
	struct threadArgs *args;	// this way keeps everything in one table
} threads;
/*---------------------------------------------------------------------------*/

/*--------------------------Functions Declaration----------------------------*/
void initialize();

int enqueue(char *QID, struct topicEntry *TE);

int getEntry(char *QID, int lastEntry, topicEntry *TE); // TE​ is an empty topicEntry​ struct

int dequeue(char *QID);

void *cleanup();

void *publisher(char *QID);

void *subsriber();
/*---------------------------------------------------------------------------*/

#endif
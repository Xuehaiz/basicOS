#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define MAXENTRIES 10
#define MAXQUEUES 10
#define URLSIZE 30
#define CAPSIZE 30


struct topicEntry {
	int entryNum;
	struct timeval timeStamp;
	int pubID;
	char photoURL[URLSIZE]; // URL to photo
	char photoCaption[CAPSIZE]; // photo caption
};

struct topicQueue {
	struct topicEntry *buffer;
	int head, tail;
	int qID;
};

struct topicQueue queues[MAXQUEUES];



int main(int argc, char const *argv[])
{
	
	return 0;
}


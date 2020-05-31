#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "InstaQuack.h"


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
			/*if (queues[i].head == 0) {
				queues[i].buffer[queues[i].length].entryNum = 0;
			}
			else {
				queues[i].buffer[queues[i].head - 1].entryNum = 0;
			}*/
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

struct topicEntry create_entry(char *photoURL, char *photoCaption) {
	struct topicEntry temp;
	temp.entryNum = 0;
	temp.pubID = 0;
	strcpy(temp.photoURL, photoURL);
	strcpy(temp.photoCaption, photoCaption);
	return temp;
}

void init() {
	for (int i = 0; i < MAXQUEUES; i++) {
		queues[i].qID = i;
		strcpy(queues[i].name,"queue");
		queues[i].head = 0;
		queues[i].tail = 0;
		queues[i].length = MAXQUEUES;
		queues[i].age = 0;
		queues[i].counter = 0;
	}
}


int isEmpty() {
	int empty = 0;
	for (int i = 0; i < MAXQUEUES; i++) {
		//printf("head: %d, tail: %d\n", queues[i].head, queues[i].tail);
		if (queues[i].head == queues[i].tail) {
			empty++;
		}
	}
	//printf("empty: %d\n", empty);
	if (empty == MAXQUEUES) {
		return 1;
	}
	return 0;
}


int main(int argc, char const *argv[])
{
	struct topicEntry te1 = create_entry("https://www.google.com/", "google");
	struct topicEntry te2 = create_entry("https://www.baidu.com/", "baidu");
	struct topicEntry te3 = create_entry("https://www.amazon.com/", "amazon");

	init();

	if (enqueue(0, te1)) {
		printf("queues[0] enqueue %s succeed\n", te1.photoCaption);
	}
	if (enqueue(0, te2)) {
		printf("queues[0] enqueue %s succeed\n", te1.photoCaption);
	}
	if (enqueue(0, te3)) {
		printf("queues[0] enqueue %s succeed\n", te1.photoCaption);
	}

	if (enqueue(1, te1)) {
		printf("queues[1] enqueue %s succeed\n", te1.photoCaption);
	}
	if (enqueue(1, te2)) {
		printf("queues[1] enqueue %s succeed\n", te1.photoCaption);
	}
	if (enqueue(1, te3)) {
		printf("queues[1] enqueue %s succeed\n", te1.photoCaption);
	}

	if (enqueue(2, te1)) {
		printf("queues[2] enqueue %s succeed\n", te1.photoCaption);
	}
	if (enqueue(2, te2)) {
		printf("queues[2] enqueue %s succeed\n", te1.photoCaption);
	}
	if (enqueue(2, te3)) {
		printf("queues[2] enqueue %s succeed\n", te1.photoCaption);
	}

	struct topicEntry TE;

	for (int i = 0; i < 3; i++) {
		
		if (isEmpty()) break;

		if (getEntry(0, 1, &TE)) {
			printf("-----Queue: 0 - Entry Num: %d - photoCaption: %s\n", TE.entryNum, TE.photoCaption);
		}

		if (getEntry(1, 2, &TE)) {
			printf("-----Queue: 0 - Entry Num: %d - photoCaption: %s\n", TE.entryNum, TE.photoCaption);
		}

		if (getEntry(2, 0, &TE)) {
			printf("-----Queue: 0 - Entry Num: %d - photoCaption: %s\n", TE.entryNum, TE.photoCaption);
		}

		// printf("here\n");
		if (dequeue(0, &TE)) {
			printf("Queue: 0 - Entry Num: %d - photoCaption: %s\n", TE.entryNum, TE.photoCaption);
		}

		if (dequeue(1, &TE)) {
			printf("Queue: 0 - Entry Num: %d - photoCaption: %s\n", TE.entryNum, TE.photoCaption);
		}

		if (dequeue(2, &TE)) {
			printf("Queue: 0 - Entry Num: %d - photoCaption: %s\n", TE.entryNum, TE.photoCaption);
		}
	}
	return 0;
}
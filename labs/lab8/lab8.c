#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define TRUE 1
#define MAXQUEUES 4
#define MAXNAME 50
#define NUMTICKET 3

struct mealTicket {
	int ticketNum;
	char dish[100];
};

struct MTQ {
	char name[MAXNAME];
	struct mealTicket buffer[5];
	int head;
	int tail;
	int length;
	int counter;
	int isfull;
	int isempty;
};

struct MTQ registry[MAXQUEUES];

const char *name_array[] = {"Breakfast", "Lunch", "Dinner", "Bar"};

void initialize() {
	for (int i = 0; i < MAXQUEUES; i++) {
		strcpy(registry[i].name, name_array[i]); // unsure
		registry[i].head = 0;
		registry[i].tail = 0;
		registry[i].counter = 0;
		registry[i].length = NUMTICKET;
		registry[i].isfull = 0;
		registry[i].isempty = 0;
	}
}

struct mealTicket MT_create(int ticketNum, char *dish) {
	struct mealTicket temp; // = (struct mealTicket*)malloc(sizeof(struct mealTicket));
	strcpy(temp.dish, dish);
	temp.ticketNum = ticketNum;
	return temp;
}

int enqueue(char *MTQ_ID, struct mealTicket MT) {
	for (int i = 0; i < MAXQUEUES; i++) {
		if (strcmp(MTQ_ID, registry[i].name) == 0) {
			if (registry[i].isfull) {  // isfull
				return 0;
			}
			// if (registry[i].buffer[registry[i].tail].ticketNum != -1) {
			MT.ticketNum = registry[i].counter;
			registry[i].counter++;
			registry[i].isempty = 0;
			registry[i].buffer[registry[i].tail] = MT;
			if (registry[i].tail + 1 == registry[i].length) {
				registry[i].tail++;
				if (registry[i].tail == registry[i].head) {
					registry[i].isfull = 1;
				}
				registry[i].tail = 0;
			}
			else {
				registry[i].tail++;
				printf("Enqueueeeeeeee head : %d, tail: %d\n", registry[i].head, registry[i].tail);
			}
			if (registry[i].tail == registry[i].head) {
				registry[i].isfull = 1;
			}
			return 1;
			// }
		}
	}
	return 0;
}

int dequeue(char *MTQ_ID, struct mealTicket *MT) { // int ticketNum,
	for (int i = 0; i < MAXQUEUES; i++) {
		if (strcmp(MTQ_ID, registry[i].name) == 0) {
			printf("Dequeue ----head : %d, tail: %d\n", registry[i].head, registry[i].tail);
			if (registry[i].isempty) {  // is empty 
				return 0;
			}
			printf("Dequeue ticketNum: %d\n", registry[i].buffer[registry[i].head].ticketNum);
			MT->ticketNum = registry[i].buffer[registry[i].head].ticketNum;
			//printf("here\n");
			strcpy(MT->dish, registry[i].buffer[registry[i].head].dish);
			registry[i].isfull = 0;
			registry[i].buffer[registry[i].head].ticketNum = -1;
			printf("-----------------------\n");
			for (int k = 0; k < NUMTICKET; k++) {
				printf("ticket Number: %d\n", registry[i].buffer[k].ticketNum);
			}
			printf("-----------------------\n");
			if (registry[i].head + 1 == registry[i].length) {
				registry[i].head++;
				if (registry[i].tail == registry[i].head) {
					registry[i].isempty = 1;
				}
				registry[i].head = 0;
			}
			else {
				registry[i].head++;
			}
			if (registry[i].tail == registry[i].head) {
					registry[i].isempty = 1;
			}
			return 1;
		}
	}
	return 2;
}

int isEmpty() {
	int empty = 0;
	for (int i = 0; i < MAXQUEUES; i++) {
		if (registry[i].head == registry[i].tail) {
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
	initialize();

	struct mealTicket mt1 = MT_create(0, "Sandwich");
	struct mealTicket mt2 = MT_create(1, "Hamburger");
	struct mealTicket mt3 = MT_create(2, "Mac-n-cheese");

	int enqueue_check = 0;

	enqueue_check = enqueue("Breakfast", mt1);
	if (enqueue_check) {
		printf("Enqueue when a queue is empty\n");
		printf("Test Case: D - Result: Success\n");
	}
	else {
		fprintf(stderr, "Breakfast enqueue Sandwich failed\n");
	}
	enqueue_check = enqueue("Breakfast", mt2);
	if (!enqueue_check) {
		fprintf(stderr, "Breakfast enqueue Hamburger failed\n");
	}
	enqueue_check = enqueue("Breakfast", mt3);
	if (!enqueue_check) {
		fprintf(stderr, "Breakfast enqueue Mac-n-cheese failed\n");
	}


	printf("======================================================\n");

	for (int i = 0; i < NUMTICKET; i++) {
		printf("registry[0].buffer[%d].ticketNum: %d   dish: %s, isfull: %d\n", 
			i, registry[0].buffer[i].ticketNum, registry[0].buffer[i].dish, registry[0].isfull);
	}
	printf("======================================================\n");


	struct mealTicket MT;
	int dequeue_check = 0;
	int first = 0;

	for (int i = 0; i < NUMTICKET; i++) {
		dequeue_check = dequeue("Breakfast", &MT);
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		if (dequeue_check) {
			printf("DeQueue: Breakfast - Ticket Number: %d - Dish: %s\n", MT.ticketNum, MT.dish);
		}
		else {
			printf("Dequeue when the queue is empty, failed.\n");
		}
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	}

	// check Dequeue when a queue is empty
/*	dequeue_check = dequeue("Breakfast", &MT); 
	if (dequeue_check) {
			printf("Queue: Breakfast - Ticket Number: %d - Dish: %s\n", MT.ticketNum, MT.dish);
	}
	else {
		printf("\n");
		printf("Check: Dequeue when a queue is empty\n");
		printf("Test Case: A - Result: Success\n");
		printf("\n");
	}*/


	enqueue_check = enqueue("Breakfast", mt1);
	if (!enqueue_check) {
		fprintf(stderr, "Breakfast enqueue Sandwich failed\n");
	}
	else {
		printf("Enqueue Sandwich Success!!! ticketNum: %d\n", registry[0].buffer[0].ticketNum);
	}
	enqueue_check = enqueue("Breakfast", mt2);
	if (!enqueue_check) {
		fprintf(stderr, "Breakfast enqueue Hamburger failed\n");
	}
	else {
		printf("Enqueue Hamburger Success!!! ticketNum: %d\n", registry[0].buffer[1].ticketNum);
	}
	enqueue_check = enqueue("Breakfast", mt3);
	if (!enqueue_check) {
		fprintf(stderr, "Breakfast enqueue Mac-n-cheese failed\n");
	}
	else {
		printf("Enqueue Mac-n-cheese Success!!! ticketNum: %d\n", registry[0].buffer[2].ticketNum);
	}


	for (int i = 0; i < NUMTICKET; i++) {
		printf("registry[0].buffer[%d].ticketNum: %d   dish: %s, isfull: %d,  isEmpty: %d\n",
			i, registry[0].buffer[i].ticketNum, registry[0].buffer[i].dish, registry[0].isfull, registry[0].isempty);
	}

	for (int i = 0; i < NUMTICKET; i++) {
		dequeue_check = dequeue("Breakfast", &MT);
		printf("dequeue ret: %d\n", dequeue_check);
		if (dequeue_check) {
			printf("isempty::::::: %d\n", registry[0].isempty);
			printf("Queue: Dinner - Ticket Number: %d - Dish: %s\n", MT.ticketNum, MT.dish);
		}
		else {
			printf("Dequeue when the queue is empty, failed.\n");
		}
	}
	printf("isempty::::::: %d\n", registry[0].isempty);
	// free(MT);
/*	free(mt1);
	free(mt2);
	free(mt3);
	for (int i = 0; i < MAXQUEUES; i++) {
		free(registry[i].buffer);
	}*/

	return 0;
}

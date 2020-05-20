/*
Description:
    CIS 415 Project 3 InstaQuack!
    quacker.h
Author: Joseph Goh
Last updated: 2019/12/08
Notes:
    N/A
TO DO:
    N/A
*/

#ifndef QUACKER_H_
#define QUACKER_H_

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "topicstore.h"

#define NUMPROXIES 5
#define MAXPUBS 20
#define MAXSUBS 20

// Thread pool struct for pubPool and subPool
typedef struct proxyPool {
    int numFiles;
    int nextFile;
    char *files[MAXPUBS];
    pthread_t threads[NUMPROXIES];
    int isFree[NUMPROXIES];
    pthread_mutex_t lock;
} proxyPool;

// Publisher proxy thread function
void *pubProxy(void *pubPool_v);

// Subscriber proxy thread function
void *subProxy(void *subPool_v);

// Old entry clean-up thread function
void *clean(void *delta_v);

// Initialize a proxyPool
int initPool(proxyPool *newPool);

// Spawn pub threads
int spawnPubs(proxyPool *pubPool);

// Spawn sub threads
int spawnSubs(proxyPool *subPool);

// Wait for proxy thread termination then destroy mutex lock
int destroyPool(proxyPool *pool);

// Parse the commands from stdin and store jobs in appropriate variables
int cmdParse(proxyPool *pubPool, proxyPool *subPool, suseconds_t *delta);

// Parse publisher command file and do publisher things
int pubParse(char *fname);

// Parse subscriber command file and do subscriber things
int subParse(char *fname);

// Builds the subscriber output html file.
void beginHTML(FILE *htmlname, char *fname);

void addToHTML(FILE *htmlptr, char *topicName, char* caption, char *url);

void finishHTML(FILE *htmlname);

int quacker();

#endif
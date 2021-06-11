#include <stdio.h>

#ifndef skipStructHeader
#define skipStructHeader
#include "../structures/skipStruct.h"
#endif

#ifndef hashTableStructHeader
#define hashTableStructHeader
#include "../structures/hashTableStruct.h"
#endif

void insertAt_skipList(Sentinel**, Record*);
void deleteFrom_skipList(Sentinel**, Record*);
int search_skipList(Node**, char**, int);
void get_Statistics(Sentinel**, char*, char*, char*, int, char**, int, int);
void printll(Node*,int);
void free_skipList(Sentinel**);
//void insertAt_linkedList(Node**, Record*, int);
//void create_skipList(Sentinel* ,Node**,int*);
#include <stdio.h>
#include <math.h>

#ifndef skipStructHeader
#define skipStructHeader
#include "../structures/skipStruct.h"
#endif

#ifndef DataStructHeader
#define DataStructHeader
#include "../structures/DataStruct.h"
#endif

#define K 16
                   
void* malloc_hook(size_t);
void* calloc_hook(size_t, size_t);
void* realloc_hook(void*, size_t);
void* allocate(void*, size_t);
void free_hook(void*);
void usleep_hook(int);
Header* makeHeader(int, int);
void write_hook(int, void*, void*, int, int);
void multiple_write_hook(int, char**, Header**, int, int, int*);
int read_hook(int, void**, int);
int virus_exists(monitorDataPointer*, char***, int*, int*);
int coinFlip();
int in_dateSpan(int, int, int, char*, int, int, int);
int in_dateSpan2(char *date1, char *date, char *date2);
int compare_dates(int, int, int, int, int, int);
float value(float);
int countDigits(int);
int isFileRead(monitorDir**, char*);
int checkForEOF(void*, int, int**);
void create_datatypes(monitorDataPointer*);
void deconstructMsg(char*, char***, int* , int SIZE);
int recentlyVacced(char*, char*);
char* findBloomFilter(parentDataPointer*, char*, char*, int*);
void addRequest(void*, char*, char*, char*, int, int);
void getRequests(void*, char*, char*, char*, char*,int);








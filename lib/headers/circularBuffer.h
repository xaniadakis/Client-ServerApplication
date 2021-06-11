#include <stdio.h>
#include <stdlib.h>

#ifndef circularBufferStructHeader
#define circularBufferStructHeader
#include "../structures/circularBufferStruct.h"
#endif

int isWritable_circularBuffer(cyclicBuffer **);
int isReadable_circularBuffer(cyclicBuffer **);
int create_circularBuffer(cyclicBuffer **, int);
int writeAt_circularBuffer(cyclicBuffer **, char *);
char *readFrom_circularBuffer(cyclicBuffer **);
void free_circularBuffer(cyclicBuffer **);
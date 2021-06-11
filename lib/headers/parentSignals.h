#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#ifndef DataStructHeader
#define DataStructHeader
#include "DataStruct.h"
#endif

void catchinterrupt(int);
void parentInterrupt(int);
int checkFlag(parentDataPointer**);

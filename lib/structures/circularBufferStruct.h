#include <stdio.h>

typedef struct cyclicBuffer{
    int cyclicBufferSize;
    int read;
    int write;
    int *r;     //tells if w[i] item of list is read
    int *w;     //tells if w[i] item of list is written
    char **buffer;
}cyclicBuffer;

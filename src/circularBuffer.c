#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef circularBufferHeader
#define circularBufferHeader
#include "../lib/headers/circularBuffer.h"
#endif

#ifndef circularBufferStructHeader
#define circularBufferStructHeader
#include "../lib/structures/circularBufferStruct.h"
#endif

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

extern pthread_mutex_t cyclicMutex;
extern pthread_cond_t cyclicCond;

// void* consume(void *arg)
// {
//     cyclicBuffer **cyclicbuffer = (cyclicBuffer **) arg;
//     pthread_mutex_lock(&cyclicMutex);
//     while(isEmpty_circularBuffer(cyclicbuffer)){
//         pthread_cond_wait(&cyclicCond,&cyclicMutex)
//     }
//     pthread_mutex_unlock(&cyclicMutex);
// }

// void* produce(void *arg)
// {
    
// }

int isWritable_circularBuffer(cyclicBuffer **cyclicbuffer)
{
    //if there is a place where parent thread can write return 1 or else return 0
    for(int i=0;i<(*cyclicbuffer)->cyclicBufferSize;i++)
        if((*cyclicbuffer)->r[i]==1 || (*cyclicbuffer)->w[i]==0)
            return 1;
    return 0;
}

int isReadable_circularBuffer(cyclicBuffer **cyclicbuffer)
{
    //if there is a place where a thread can read return 1 or else return 0
    for(int i=0;i<(*cyclicbuffer)->cyclicBufferSize;i++)
        if((*cyclicbuffer)->r[i]==0 && (*cyclicbuffer)->w[i]==1)
            return 1;
    return 0;
}

int create_circularBuffer(cyclicBuffer **cyclicbuffer, int cyclicBufferSize)
{
    (*cyclicbuffer) = (cyclicBuffer*) malloc_hook(sizeof(cyclicBuffer));
    (*cyclicbuffer)->buffer = (char**) malloc_hook(sizeof(char*)*cyclicBufferSize);
    (*cyclicbuffer)->r = (int*) malloc_hook(sizeof(int)*cyclicBufferSize);
    (*cyclicbuffer)->w = (int*) malloc_hook(sizeof(int)*cyclicBufferSize);
    for(int i=0;i<cyclicBufferSize;i++){
        (*cyclicbuffer)->buffer[i] = NULL;
        (*cyclicbuffer)->r[i] = 0;
        (*cyclicbuffer)->w[i] = 0;
    }
    (*cyclicbuffer)->cyclicBufferSize = cyclicBufferSize;
    (*cyclicbuffer)->write = 0;
    (*cyclicbuffer)->read = 0;
}

int writeAt_circularBuffer(cyclicBuffer **cyclicbuffer, char *path)
{
    int retval = (*cyclicbuffer)->write;

    //copy the path string in the array at the writeIndex item
    free_hook((*cyclicbuffer)->buffer[(*cyclicbuffer)->write]);
    (*cyclicbuffer)->buffer[(*cyclicbuffer)->write] = malloc_hook(strlen(path)+1);
    strcpy((*cyclicbuffer)->buffer[(*cyclicbuffer)->write], path);
    
    //define this new item as written and not read
    (*cyclicbuffer)->r[(*cyclicbuffer)->write] = 0;
    (*cyclicbuffer)->w[(*cyclicbuffer)->write] = 1;
    
    //move writeIndex to the next item
    //if writeIndex equals readIndex that means, move readIndex one item later 
    //because buffer is FIFO and you dont want reader to read the last item
    //if next item does not exist move to the first item 
    // if((*cyclicbuffer)->write==(*cyclicbuffer)->read)
    // {
    //     ((*cyclicbuffer)->write)++;
    //     ((*cyclicbuffer)->read)++;
    //     if((*cyclicbuffer)->write>=(*cyclicbuffer)->cyclicBufferSize){
    //         (*cyclicbuffer)->write=0;
    //         (*cyclicbuffer)->read=0;
    //     }
    // }
    // else{
        ((*cyclicbuffer)->write)++;
        if((*cyclicbuffer)->write>=(*cyclicbuffer)->cyclicBufferSize)
            (*cyclicbuffer)->write=0;
    // }

    //return which item you wrote to the caller
    return retval;
}

char *readFrom_circularBuffer(cyclicBuffer **cyclicbuffer)
{
    //define this item as read
    (*cyclicbuffer)->r[(*cyclicbuffer)->read] = 1;
    
    //move to the next item 
    ((*cyclicbuffer)->read)++;
    if((*cyclicbuffer)->read>=(*cyclicbuffer)->cyclicBufferSize)
        (*cyclicbuffer)->read=0;
    
    //read the previous item and return it to the caller
    if((*cyclicbuffer)->read==0)
        return (*cyclicbuffer)->buffer[(*cyclicbuffer)->cyclicBufferSize-1];
    else
        return (*cyclicbuffer)->buffer[(*cyclicbuffer)->read-1];
}

void free_circularBuffer(cyclicBuffer **cyclicbuffer)
{
    for(int i=0;i<(*cyclicbuffer)->cyclicBufferSize;i++)
        free_hook((*cyclicbuffer)->buffer[i]);
    free((*cyclicbuffer)->r);
    free((*cyclicbuffer)->w);
    free((*cyclicbuffer)->buffer);
    //free((*cyclicbuffer));
}





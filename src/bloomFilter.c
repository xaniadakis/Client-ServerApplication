#include <stdio.h>
#include <stdlib.h>

#ifndef hashFunctionsHeader
#define hashFunctionsHeader
#include "../lib/headers/hashFunctions.h"
#endif

#ifndef DataStructHeader
#define DataStructHeader
#include "../lib/structures/DataStruct.h"
#endif

void insertAt_bloomFilter(char **bloomFilter, int N, int bloomSize, unsigned char *item)
{
    int bit, nbyte, nbit;
    for(int i=0; i<K; i++)
    {
        bit = hash_i(item, i)%bloomSize;
        nbyte = bit/8;
        nbit = bit%8;
        //printf("pos=%d & byte=%d & bit=%d \n",bit,nbyte,nbit);
        if(nbit==0)
            nbit=-128;
        else if(nbit==1)
            nbit=64;
        else if(nbit==2)
            nbit=32;
        else if(nbit==3)
            nbit=16;
        else if(nbit==4)
            nbit=8;
        else if(nbit==5)
            nbit=4;
        else if(nbit==6)
            nbit=2;
        else if(nbit==7)
            nbit=1;
        //printf("before '%d'\n",bloomFilter[N][nbyte]);
        bloomFilter[N][nbyte] = (bloomFilter[N][nbyte]) | nbit;
        //printf("after'%d'\n",bloomFilter[N][nbyte]);

    }
}

int isVaccinated_bloomFilter(char *bloomFilter, int bloomSize, unsigned char *item)
{
    int bit, nbyte, nbit;
    char byte;
    int count=0;
    for(int i=0; i<K; i++)
    {
        bit = hash_i(item, i)%bloomSize;
        nbyte = bit/8;
        nbit = bit%8;
        byte=bloomFilter[nbyte];
        if(nbit==0)
            byte=byte>>7;
        else if(nbit==1){
            byte=byte<<1;
            byte=byte>>7;
        }
        else if(nbit==2){
            byte=byte<<2;
            byte=byte>>7;
        }
        else if(nbit==3){
            byte=byte<<3;
            byte=byte>>7;
        }
        else if(nbit==4){
            byte=byte<<4;
            byte=byte>>7;
        }
        else if(nbit==5){
            byte=byte<<5;
            byte=byte>>7;
        }
        else if(nbit==6){
            byte=byte<<6;
            byte=byte>>7;
        }
        else if(nbit==7){
            byte=byte<<7;
            byte=byte>>7;
        }
        count=count+byte; 
    }
    if(count==-K)
        return 1;
    else{
        //printf("'%d'\n",bloomFilter[N][nbyte]);
        //printf("findings=%d\n",count);
        return 0;
    }
}

int isItemIn_bloomFilter(char **bloomFilter, int N, int bloomSize, unsigned char *item)
{
    int bit, nbyte, nbit;
    char byte;
    int count=0;
    for(int i=0; i<K; i++)
    {
        bit = hash_i(item, i)%bloomSize;
        nbyte = bit/8;
        nbit = bit%8;
        byte=bloomFilter[N][nbyte];
        if(nbit==0)
            byte=byte>>7;
        else if(nbit==1){
            byte=byte<<1;
            byte=byte>>7;
        }
        else if(nbit==2){
            byte=byte<<2;
            byte=byte>>7;
        }
        else if(nbit==3){
            byte=byte<<3;
            byte=byte>>7;
        }
        else if(nbit==4){
            byte=byte<<4;
            byte=byte>>7;
        }
        else if(nbit==5){
            byte=byte<<5;
            byte=byte>>7;
        }
        else if(nbit==6){
            byte=byte<<6;
            byte=byte>>7;
        }
        else if(nbit==7){
            byte=byte<<7;
            byte=byte>>7;
        }
        count=count+byte; 
    }
    if(count==-K)
        return 1;
    else{
        //printf("'%d'\n",bloomFilter[N][nbyte]);
        //printf("findings=%d\n",count);
        return 0;
    }
}

void free_bloomFilter(char ***bloomFilter, int N)
{
    for(int i=0; i<N; i++){
        free((*bloomFilter)[i]);
    }
    free(*bloomFilter);
}

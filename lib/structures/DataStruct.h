#include <stdio.h>
#include <arpa/inet.h>
#include <signal.h>

#ifndef skipStructHeader
#define skipStructHeader
#include "skipStruct.h"
#endif

#ifndef hashTableStructHeader
#define hashTableStructHeader
#include "hashTableStruct.h"
#endif

#ifndef RequestStructHeader
#define RequestStructHeader
#include "requestStruct.h"
#endif

#ifndef moreStructsHeader
#define moreStructsHeader
#include "moreStructs.h"
#endif

#ifndef circularBufferStructHeader
#define circularBufferStructHeader
#include "circularBufferStruct.h"
#endif

typedef struct parentDataPointer{
    reqReg **requestsRegistryPtr;
    int *numMonitorsPtr;
    int *numThreadsPtr;
    char **dirnamePtr;
    char ***argumentPtr;
    int *bloomSizePtr;
    int *socketBufferSizePtr;
    int *cyclicBufferSizePtr;
    Monitor ***monitorPtr;
    struct sigaction *actPtr;
    int **fifoPtr;
    int **socketPtr;
    int PORT;
    char ***namedPipePtr;
}parentDataPointer;

typedef struct monitorDataPointer{
    reqReg **requestsRegistryPtr;
    Record ***RegistryPtr;
    Sentinel ***skipListPtr;
    cyclicBuffer **cyclicbufferPtr;    
    char ***bloomFilterPtr;
    char ***countryDataPtr;
    char ***virusDataPtr;
    char ***vaccedPtr;
    char ***argvPtr;
    char **dirnamePtr;
    int *socketBufferSizePtr;
    int *cyclicBufferSizePtr;
    int *NbloomFiltersPtr;
    int *NskipListsPtr;
    int *bloomSizePtr;
    int *NcountriesPtr;
    int *inputSizePtr;
    monitorDir **mySubDirPtr;
    int **fifoPtr;
    int *mySocketPtr;
    int *myPortPtr;
    int *numThreadsPtr;
    pthread_t **parserPtr;
    int initSL;
}monitorDataPointer;



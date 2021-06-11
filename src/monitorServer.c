#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/select.h>
#include <ctype.h>
#include <netdb.h>
#include <pthread.h>

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef DataStructHeader
#define DataStructHeader
#include "../lib/structures/DataStruct.h"
#endif

#ifndef hashTableHeader
#define hashTableHeader
#include "../lib/headers/hashTable.h"
#endif

#ifndef skipListHeader
#define skipListHeader
#include "../lib/headers/skipList.h"
#endif

#ifndef monitorUtilHeader
#define monitorUtilHeader
#include "../lib/headers/monitorServerUtil.h"
#endif

#ifndef hashFunctionsHeader
#define hashFunctionsHeader
#include "../lib/headers/hashFunctions.h"
#endif

#ifndef signalsHeader
#define signalsHeader
#include "../lib/headers/monitorSignals.h"
#endif

#define READ 0
#define WRITE 1
#define _XOPEN_SOURCE 700

int FLAGINTQUIT = 0;
int FLAGUSR1 = 0;
int FLAGUSR2 = 0;
int FLAGCONT = 0;

int main(int argc, char *argv[])
{
    // printf("Process %d",getpid());
    
    // for(int i=1; i<argc; i++)
    //     printf(" %s",argv[i]);

    // printf("\n");

    static struct sigaction act;
    static struct sigaction act2;
    act2.sa_handler = monitorThreadInterrupt;
    // static struct sigaction act1;
	// act1.sa_handler = monitorInterrupt;
    act.sa_handler=SIG_DFL; 
    // act.sa_flags = SA_RESTART;
	sigfillset(&(act.sa_mask));
	sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    // sigaction(SIGUSR1, &act1, NULL);
    sigaction(SIGUSR1, &act2, NULL);
    sigaction(SIGUSR2, &act2, NULL);

    
    //dataTypes
    monitorDataPointer *Universal = NULL;       //a struct that keeps pointers to the important data of the program   
    Record **Registry = NULL;                   //hashTable dataType
    Sentinel **skipList = NULL;                 //skipList dataType
    char **bloomFilter = NULL;                  //bloomFilter dataType 
    reqReg *requestsRegistry = NULL;            //a struct to save the requests happening
    monitorDir *mySubDir = NULL;                //a struct to save info about files and sub_dirs assigned to this monitor

    //pluralities
    int sizeOfBloom = 0;                        //size of bloomFilter in bytes (defined by the user)
    int inputSize = 0;                          //plurality of citizens kept in the dataBase (no duplicates)
    int NskipLists = 0;                         //plurality of skipLists
    int NbloomFilters = 0;                      //plurality of bloomFilters == plurality of viruses stored in array virusData
    int Ncountries = 0;                         //plurality of countries stored in array countryData
    int socketBufferSize = 0;                   //size of buffer for read and write on sockets
    int cyclicBufferSize = 0;                   //cyclic buffer size
    int myPort = atoi(argv[2]);
    int mySocket = 0;
    int numThreads = 0;

    //arrays that store data to avoid information reoccurrence
    char **virusData = NULL;                    //virusData array keeps track of the viruses that exist
    char **countryData = NULL;                  //countryData array keeps track of the countries that exist
    char **vacced = NULL;                       //vacced array has two items "YES" and "NO"
    char *input_dir = NULL;      

    //named pipes
    int *fifo = NULL;

    pthread_t parentThread = NULL;
    cyclicBuffer *cyclicbuffer = NULL;
    pthread_t *parser = NULL;

    //save pointers to all the important data of the program in a struct for easy access
    uploadMonitorData(&Universal, &Registry, &skipList, &bloomFilter, &countryData,
                       &virusData, &vacced, &input_dir, &socketBufferSize, &cyclicBufferSize, 
                       &NbloomFilters, &NskipLists, &sizeOfBloom, &Ncountries, &inputSize, 
                       &mySubDir, &fifo, &mySocket, &myPort, &numThreads, &requestsRegistry, &argv,
                       &cyclicbuffer, &parser);
    
    parametersCheck(Universal, argc, argv);

    pthread_create(&parentThread, NULL, Main, &Universal);
    pthread_join(parentThread, NULL);
    // Main(&Universal);
    // exitNow(&Universal);
    pthread_exit(NULL);
}
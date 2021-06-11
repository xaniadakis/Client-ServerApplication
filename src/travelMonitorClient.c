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
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>


#define FAILURE 1
#define SUCCESS 0
#define FIFO_DIR "../tmp/namedPipes/"
#define LOG_DIR "../log_folder/"
#define _XOPEN_SOURCE 700

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef travelMonitorClientUtilHeader
#define travelMonitorClientUtilHeader
#include "../lib/headers/travelMonitorClientUtil.h"
#endif

#ifndef signalsHeader
#define signalsHeader
#include "../lib/headers/parentSignals.h"
#endif

extern int errno;
int FLAGINTQUIT = 0;
int FLAGCHLD = 0;
int FLAGUSR2 = 0;

int main(int argc, char *argv[])
{
    //clear terminal
    // system("clear");
    printf("\033[1;39mWelcome to the travelMonitorClient app!\n\033[0m");        

    parentDataPointer *Universal = NULL;   
    reqReg *requestsRegistry = NULL; 
    Monitor **monitor = NULL; 
    char *input_dir = NULL;
    char **namedPipe = NULL; 
    char **argument = NULL; 
    int *fifo = NULL; 
    int *socket = NULL;
    int sizeOfBloom = 0;                          
    int numMonitors = 0;
    int numThreads = 0;
    int socketBufferSize = 0;
    int cyclicBufferSize = 0;
    
    static struct sigaction act;
    static struct sigaction act1;
	act1.sa_handler = parentInterrupt;
    act1.sa_flags = SA_RESTART;
    act.sa_handler=SIG_DFL; 
	sigfillset(&(act.sa_mask));
	sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGUSR2, &act1, NULL);

    //keep pointers to the important data of the program in a struct 
    uploadParentData(&Universal, &numMonitors, &numThreads, &input_dir, &sizeOfBloom, 
                     &socketBufferSize, &cyclicBufferSize, &monitor, &act1, &fifo, 
                     &socket, &namedPipe, &requestsRegistry, &argument);

    //check the parameters for correctness     
    parametersCheck(Universal, argc, argv);
    printf("\n     \x1B[7mapplication data\x1B[0m             \x1B[1m---->\x1B[0m");
    printf("     \x1B[7mnumMonitors:\x1B[0m \x1B[1m%d\x1B[0m         \x1B[7msocketBufferSize:\x1B[0m \x1B[1m%d\x1B[0m               \x1B[7mcyclicBufferSize:\x1B[0m \x1B[1m%d\x1B[0m",numMonitors, socketBufferSize, cyclicBufferSize);
    printf("\n\n                                  \x1B[1m---->\x1B[0m");
    printf("     \x1B[7msizeOfBloom:\x1B[0m \x1B[1m%d\x1B[0m     \x1B[7minput_dir:\x1B[0m \x1B[1m\"%s\"\x1B[0m     \x1B[7mnumThreads:\x1B[0m \x1B[1m%d\x1B[0m\
            \n\n", sizeOfBloom, input_dir, numThreads);
    if(*(Universal->numMonitorsPtr)>1)
        printf("\033[3;39mAttempting to connect to the monitorServers...\033[0m\n");
    else if (*(Universal->numMonitorsPtr)==1)
        printf("\033[3;39mAttempting to connect to the monitorServer...\033[0m\n");
    else{
        printf("\033[3;39mThe travelMonitorClient could not connect to a monitorServer.\033[0m\n");
        exit(EXIT_FAILURE);
    }

    //do some initializations
    initMonitorDetails(Universal);

    //delete temp files from previous execution
    deleteOldLogFiles();

    //create named pipes
    createSockets(Universal);

    //distribute the subdir from the given input_dir to the proccesses using RR
    distributeSubdirs(Universal, -1);

    //create proccesses
    for(int i=0;i<numMonitors;i++) 
        fork_monitor(Universal, i);

    //wait while monitor proccesses create the datatypes
    usleep_hook(110000);     
    //receive the bloomfilters from monitor proccesses
    for(int i=0;i<numMonitors;i++)
        receiveBloomFilter(Universal, i);

    //receive ready message from monitors
    for(int i=0;i<numMonitors;i++)
        sendReadyMsg(Universal, i);
    for(int i=0;i<numMonitors;i++)
    {
        while(!receiveReadyMsg(Universal, i));
            //printf("waiting for readymsg from %d\n",monitor[i]->pid);
    }

    //wait for user input from keyboard 
    readInputFromKeyboard(Universal);

    //delete temp files
    rmdir("../tmp/namedPipes");
    rmdir("../tmp");

    //exit normally
    return EXIT_SUCCESS;
}

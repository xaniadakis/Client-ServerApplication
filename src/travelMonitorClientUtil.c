#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <math.h>
#include <sys/select.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <ctype.h>
#include <sys/errno.h>
#include <signal.h>
#include <netdb.h>

#define _XOPEN_SOURCE 700
#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define YELLOW  "\x1B[38;2;255;165;0m"
#define RESET   "\x1b[0m"

#ifndef travelMonitorClientUtilHeader
#define travelMonitorClientUtilHeader
#include "../lib/headers/travelMonitorClientUtil.h"
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

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef bloomFilterHeader
#define bloomFilterHeader
#include "../lib/headers/bloomFilter.h"
#endif

#ifndef hashFunctionsHeader
#define hashFunctionsHeader
#include "../lib/headers/hashFunctions.h"
#endif

#ifndef signalsHeader
#define signalsHeader
#include "../lib/headers/parentSignals.h"
#endif

#define K 16
#define FIFO_DIR "../tmp/namedPipes/"
#define LOG_DIR "../log_folder/"

extern int errno;
extern int FLAGCHLD;

//stores pointers to the important data of the program to a single struct so that access to them can be easily accomplished
void uploadParentData(parentDataPointer **Universal, int *numMonitors, int *numThreads, char **dirname, int *bloomSize, int *socketBufferSize, 
                      int *cyclicBufferSize, Monitor ***monitor, struct sigaction *act, int **fifo, int **socket, char ***namedPipe, 
                      reqReg **requestsRegistry, char ***argument)
{
    (*Universal) = (parentDataPointer*) malloc(sizeof(parentDataPointer));
    (*Universal)->numMonitorsPtr = numMonitors;
    (*Universal)->numThreadsPtr = numThreads;
    (*Universal)->dirnamePtr = dirname;
    (*Universal)->bloomSizePtr = bloomSize;
    (*Universal)->socketBufferSizePtr = socketBufferSize;
    (*Universal)->cyclicBufferSizePtr = cyclicBufferSize;
    (*Universal)->monitorPtr = monitor;
    (*Universal)->actPtr = act;
    (*Universal)->fifoPtr = fifo;
    (*Universal)->socketPtr = socket;
    (*Universal)->namedPipePtr = namedPipe;
    (*Universal)->requestsRegistryPtr = requestsRegistry;
    (*Universal)->argumentPtr = argument;

    (*requestsRegistry) = malloc_hook(sizeof(reqReg));
    (*requestsRegistry)->n = 0;
    (*requestsRegistry)->array = NULL;

    (*argument) = (char**) malloc_hook(sizeof(char*)*1);
    (*argument)[0] = (char*) malloc_hook(sizeof(char)*(strlen("./monitorServer")+1));
    strcpy((*argument)[0],"./monitorServer");

    // (*Requests) = malloc_hook(1*sizeof(requests));
    // (*Requests)->countryTo = NULL;
    // (*Requests)->overallRequests = 0;
    // (*Requests)->overallAcceptedRequests = 0; 
    // (*Requests)->overallRejectedRequests = 0;
    // (*Requests)->totalTravelRequestsDate = NULL;
    // (*Requests)->totalTravelRequestsCtr = NULL;
    // (*Requests)->acceptedRequestsDate = NULL;
    // (*Requests)->acceptedRequestsCtr = NULL;
    // (*Requests)->rejectedRequestsDate = NULL;
    // (*Requests)->rejectedRequestsCtr = NULL;

    // (*monitor) = (Monitor*) malloc_hook(sizeof(Monitor)*(*numMonitors));
    // for(int i=0; i<(*numMonitors);i++)
    // {
    //     ((*monitor)[i]).Nsub_dirs = 0;
    //     ((*monitor)[i]).NbloomFilters = 0;
    //     ((*monitor)[i]).sub_dir = NULL;
    //     ((*monitor)[i]).bloomFilter = NULL;
    // }
    // (*namedPipe) = (char**) malloc_hook(sizeof(char*)*(*numMonitors)*2);
    // (*fifo) = (int*) malloc_hook(sizeof(int)*(*numMonitors)*2);
}

void parametersCheck(parentDataPointer* Universal, int argc, char **argv)
{
    int *numThreads = Universal->numThreadsPtr;
    int *numMonitors = Universal->numMonitorsPtr;
    int *bloomSize = Universal->bloomSizePtr;
    int *socketBufferSize = Universal->socketBufferSizePtr;
    char **dirname = Universal->dirnamePtr;
    int *cyclicBufferSize = Universal->cyclicBufferSizePtr;

    
    if(argc<13){
        printf("Wrong number of parameters %d\n./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads\n",argc);
        int counter=0;
        while(counter<argc)
            printf("%s ",argv[counter++]);
        printf("\n");
        exit(1);
    }

    if(!strcmp("-m", argv[1]))
        (*numMonitors)=atoi(argv[2]);
    else if(!strcmp("-m", argv[3]))
        (*numMonitors)=atoi(argv[4]);
    else if(!strcmp("-m", argv[5]))
        (*numMonitors)=atoi(argv[6]);
    else if(!strcmp("-m", argv[7]))
        (*numMonitors)=atoi(argv[8]);
    else if(!strcmp("-m", argv[9]))
        (*numMonitors)=atoi(argv[10]);
    else if(!strcmp("-m", argv[11]))
        (*numMonitors)=atoi(argv[12]);
    else{
        printf("Wrong parameters\n./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads\n");
        exit(1);
    }

    if(!strcmp("-b", argv[1]))
        (*socketBufferSize)=atoi(argv[2]);
    else if(!strcmp("-b", argv[3]))
        (*socketBufferSize)=atoi(argv[4]);
    else if(!strcmp("-b", argv[5]))
        (*socketBufferSize)=atoi(argv[6]);
    else if(!strcmp("-b", argv[7]))
        (*socketBufferSize)=atoi(argv[8]);
    else if(!strcmp("-b", argv[9]))
        (*socketBufferSize)=atoi(argv[10]);
    else if(!strcmp("-b", argv[11]))
        (*socketBufferSize)=atoi(argv[12]);
    else{
        printf("Wrong parameters\n./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads\n");
        exit(1);
    }

    if(!strcmp("-c", argv[1]))
        (*cyclicBufferSize)=atoi(argv[2]);
    else if(!strcmp("-c", argv[3]))
        (*cyclicBufferSize)=atoi(argv[4]);
    else if(!strcmp("-c", argv[5]))
        (*cyclicBufferSize)=atoi(argv[6]);
    else if(!strcmp("-c", argv[7]))
        (*cyclicBufferSize)=atoi(argv[8]);
    else if(!strcmp("-c", argv[9]))
        (*cyclicBufferSize)=atoi(argv[10]);
    else if(!strcmp("-c", argv[11]))
        (*cyclicBufferSize)=atoi(argv[12]);
    else{
        printf("Wrong parameters\n./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads\n");
        exit(1);
    }

    if(!strcmp("-s", argv[1]))
        (*bloomSize)=atoi(argv[2]);
    else if(!strcmp("-s", argv[3]))
        (*bloomSize)=atoi(argv[4]);
    else if(!strcmp("-s", argv[5]))
        (*bloomSize)=atoi(argv[6]);
    else if(!strcmp("-s", argv[7]))
        (*bloomSize)=atoi(argv[8]);
    else if(!strcmp("-s", argv[9]))
        (*bloomSize)=atoi(argv[10]);
    else if(!strcmp("-s", argv[11]))
        (*bloomSize)=atoi(argv[12]);
    else{
        printf("Wrong parameters\n./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads\n");
        exit(1);
    }

    if(!strcmp("-i", argv[1])){
        (*dirname)=(char*) malloc(sizeof(char)*(strlen(argv[2])+1));
        strcpy((*dirname), argv[2]);
    }
    else if(!strcmp("-i", argv[3])){
        (*dirname)=(char*) malloc(sizeof(char)*(strlen(argv[4])+1));
        strcpy((*dirname), argv[4]);
    }
    else if(!strcmp("-i", argv[5])){
        (*dirname)=(char*) malloc(sizeof(char)*(strlen(argv[6])+1));
        strcpy((*dirname), argv[6]);
    }
    else if(!strcmp("-i", argv[7])){
        (*dirname)=(char*) malloc(sizeof(char)*(strlen(argv[8])+1));
        strcpy((*dirname), argv[8]);
    }
    else if(!strcmp("-i", argv[9])){
        (*dirname)=(char*) malloc(sizeof(char)*(strlen(argv[10])+1));
        strcpy((*dirname), argv[10]);
    }
    else if(!strcmp("-i", argv[11])){
        (*dirname)=(char*) malloc(sizeof(char)*(strlen(argv[12])+1));
        strcpy((*dirname), argv[12]);
    }
    else{
        printf("Wrong parameters\n./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads\n");
        exit(1);
    }

    if(!strcmp("-t", argv[1]))        
        (*numThreads)=atoi(argv[2]);
    else if(!strcmp("-t", argv[3]))        
        (*numThreads)=atoi(argv[4]);
    else if(!strcmp("-t", argv[5]))        
        (*numThreads)=atoi(argv[6]);
    else if(!strcmp("-t", argv[7]))        
        (*numThreads)=atoi(argv[8]);
    else if(!strcmp("-t", argv[9]))        
        (*numThreads)=atoi(argv[10]);
    else if(!strcmp("-t", argv[11]))        
        (*numThreads)=atoi(argv[12]);
    else{
        printf("Wrong parameters\n./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads\n");
        exit(1);
    }
    srand(time(NULL)); //initialize pseudo-random number generator for coinFlip function that will be used in the skipList creation
}

void initMonitorDetails(parentDataPointer* Universal)
{
    Monitor ***monitor = Universal->monitorPtr;
    int numMonitors = *(Universal->numMonitorsPtr);
    (*monitor) = (Monitor**) malloc_hook(sizeof(Monitor*)*numMonitors);
    for(int i=0; i<numMonitors;i++)
    {
        (*monitor)[i] = (Monitor*) malloc_hook(sizeof(Monitor));
        (*monitor)[i]->pid = 0;
        (*monitor)[i]->Nsub_dirs = 0;
        (*monitor)[i]->NbloomFilters = 0;
        (*monitor)[i]->sub_dir = NULL;
        (*monitor)[i]->bloomFilter = NULL;
    }
    // *(Universal->namedPipePtr) = (char**) malloc_hook(sizeof(char*)*numMonitors*2);
    // *(Universal->fifoPtr) = (int*) malloc_hook(sizeof(int)*numMonitors*2);
    *(Universal->socketPtr) = (int*) malloc_hook(sizeof(int)*numMonitors);
    Universal->PORT = (rand()%(65535-49152))+49152;
}

void deleteOldLogFiles()
{
    struct dirent *file;
    DIR *dir;
    char *filename = malloc_hook(1);
    dir = opendir(LOG_DIR);
    if (dir != NULL)
    {    
        while((file=readdir(dir)) != NULL)
        if(strcmp(file->d_name,".") && strcmp(file->d_name,"..") && file->d_ino!=0){
            filename = (char*) realloc(filename, strlen(LOG_DIR)+strlen(file->d_name)+1); 
            snprintf(filename, strlen(LOG_DIR)+strlen(file->d_name)+1 , "%s%s", LOG_DIR, file->d_name);
            remove(filename);
        }
        closedir(dir);
    }
    free(filename);
    rmdir(LOG_DIR);
}

void createSockets(parentDataPointer *Universal)
{
    // char **namedPipe = *(Universal->namedPipePtr);
    int numMonitors = *(Universal->numMonitorsPtr); 
    // mkdir("../tmp", 0777);
    // mkdir(FIFO_DIR, 0777);
    // for(int i=0;i<numMonitors*2;i++) 
    // {
    //     if(i%2==0){
    //         namedPipe[i] = (char*) malloc_hook(sizeof(char)*(strlen(FIFO_DIR)+19));
    //         snprintf(namedPipe[i], strlen(FIFO_DIR)+18 , FIFO_DIR"pipe%d_parent.fifo", i/2+1);
    //         mkfifo(namedPipe[i],0666);
    //         if(mkfifo(namedPipe[i],0666)<0 && errno!=EEXIST){
    //             for(int j=0;j<i;j++) 
    //                 unlink(namedPipe[j]);
    //             perror("fifo");
    //             exit(EXIT_FAILURE); 
    //         }
    //     }else if(i%2==1){
    //         namedPipe[i] = (char*) malloc_hook(sizeof(char)*(strlen(FIFO_DIR)+18));
    //         snprintf(namedPipe[i], strlen(FIFO_DIR)+18 , FIFO_DIR"pipe%d_child.fifo", i/2+1);
    //         if(mkfifo(namedPipe[i],0666)<0 && errno!=EEXIST){
    //             for(int j=0;j<i;j++) 
    //                 unlink(namedPipe[j]);
    //             perror("fifo");
    //             exit(EXIT_FAILURE); 
    //         }
    //     }
    // }
    int *socketFd = *(Universal->socketPtr);
    struct sockaddr_in address;
    int opt = 1;
    int PORT = Universal->PORT;

    struct hostent *host_entry;
    char hostname[HOST_NAME_MAX+1];
    char *myIPaddress;
    if(gethostname(hostname, HOST_NAME_MAX)==-1)
    {
        perror("gethostname");
        exit(EXIT_FAILURE);
    }
    if((host_entry = gethostbyname(hostname))==NULL)
    {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }
    int counter=0;
    while(1)
    {
        if(host_entry->h_addr_list[counter]!=NULL)
            myIPaddress = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[counter]));   
        else 
            break;
        counter++;
    }
    if(myIPaddress==NULL)
    {
        perror("IP address");
        exit(EXIT_FAILURE);
    }
    printf("\n     \x1B[7mtravelMonitorClient %d\x1B[0m    \x1B[1m---->\x1B[0m     \x1B[7mIP address:\x1B[0m \x1B[1m%s\x1B[0m       ",getpid(),myIPaddress);
    printf("\x1B[7mPorts:\x1B[0m \x1B[1m%d - %d\x1B[0m\n", Universal->PORT, Universal->PORT + *(Universal->numMonitorsPtr) - 1);

    for(int i=0;i<numMonitors;i++) 
    {    
        if(!(socketFd[i] = socket(AF_INET, SOCK_STREAM, 0)))
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        if(setsockopt(socketFd[i], SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        memset((char *) &address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(myIPaddress);
        address.sin_port = htons(PORT+i);

        if(bind(socketFd[i], (struct sockaddr *)&address, sizeof(address))<0)
        {
            perror("bind");
            close(socketFd[i]);
            exit(EXIT_FAILURE);
        }
        if(listen(socketFd[i], 5) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    }
}

void fork_monitor(parentDataPointer *Universal, int i)
{
    // int *fifo = *(Universal->fifoPtr); 
    char ***argument = Universal->argumentPtr;
    Monitor **monitor = *(Universal->monitorPtr);
    (*argument) = realloc_hook((*argument), sizeof(char*)*(12+monitor[i]->Nsub_dirs));
    int *socketFd = *(Universal->socketPtr);
    int mySocket;
    struct sockaddr_in address;
    memset((char *) &address, 0, sizeof(address));
    int addrLength = sizeof(address);
    // char **namedPipe = *(Universal->namedPipePtr);
    monitor[i]->pid = fork();        
    if(monitor[i]->pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(monitor[i]->pid == 0){
        (*argument)[1] = malloc_hook(3);
        strcpy((*argument)[1],"-p");

        (*argument)[2] = malloc_hook(6);
        snprintf((*argument)[2], 6, "%d",  Universal->PORT+i);

        (*argument)[3] = malloc_hook(3);
        strcpy((*argument)[3],"-t");

        (*argument)[4] = malloc_hook(countDigits(*(Universal->numThreadsPtr))+1);
        snprintf((*argument)[4], *(Universal->numThreadsPtr)+1, "%d", *(Universal->numThreadsPtr));

        (*argument)[5] = malloc_hook(3);
        strcpy((*argument)[5],"-b");

        (*argument)[6] = malloc_hook(countDigits(*(Universal->socketBufferSizePtr))+1);
        snprintf((*argument)[6], *(Universal->socketBufferSizePtr)+1, "%d", *(Universal->socketBufferSizePtr));

        (*argument)[7] = malloc_hook(3);
        strcpy((*argument)[7],"-c");

        (*argument)[8] = malloc_hook(countDigits(*(Universal->cyclicBufferSizePtr))+1);
        snprintf((*argument)[8], *(Universal->cyclicBufferSizePtr)+1, "%d", *(Universal->cyclicBufferSizePtr));

        (*argument)[9] = malloc_hook(3);
        strcpy((*argument)[9],"-s");

        (*argument)[10] = malloc_hook(countDigits(*(Universal->bloomSizePtr))+1);
        snprintf((*argument)[10], *(Universal->bloomSizePtr)+1, "%d", *(Universal->bloomSizePtr));


        for(int k=0;k<monitor[i]->Nsub_dirs;k++){
            (*argument)[11+k] = malloc_hook(strlen(monitor[i]->sub_dir[k])+1);            
            strcpy((*argument)[11+k], monitor[i]->sub_dir[k]);           
        }

        (*argument)[11+monitor[i]->Nsub_dirs] = NULL;
        execv((*argument)[0],(*argument));
        // printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid());
        // exit(SUCCESS);
    }
    else if(monitor[i]->pid > 0){
        // fifo[2*i] = open(namedPipe[2*i], O_WRONLY);
        // if(fifo[2*i]<0) 
        //     perror("fifo");
        // fifo[2*i+1] = open(namedPipe[2*i+1], O_RDONLY);
        // if(fifo[2*i+1]<0) 
        //     perror("fifo");
        if((mySocket = accept(socketFd[i], (struct sockaddr*) &address, (socklen_t*) &addrLength))<0)
        {
            perror("accept");
            if(errno==EINTR){
                if((mySocket = accept(socketFd[i], (struct sockaddr*) &address, (socklen_t*) &addrLength))<0)
                {
                    perror("accept");                       
                    exit(EXIT_FAILURE);
                }
            }
            exit(EXIT_FAILURE);
        }
        shutdown(socketFd[i], SHUT_RDWR);
        close(socketFd[i]);      
        socketFd[i] = mySocket;
        write_hook(socketFd[i], *(Universal->dirnamePtr), makeHeader(0, strlen(*(Universal->dirnamePtr))+1), *(Universal->socketBufferSizePtr), strlen(*(Universal->dirnamePtr))+1);
    }
    // free(yourPort);
}

void passInfo(parentDataPointer *Universal, int monitor)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    // int *fifo = *(Universal->fifoPtr); 
    char *dirname = *(Universal->dirnamePtr);
    int bloomSize = *(Universal->bloomSizePtr); 
    int *socketFd = *(Universal->socketPtr);
    Monitor **thismonitor = *(Universal->monitorPtr);
    void *dataBuffer = malloc_hook(1);
    argvs *pass_arg = (argvs*) malloc_hook(sizeof(argvs));
    pass_arg->bloomSize = bloomSize;
    pass_arg->bufferSize = socketBufferSize;
    

    // write_hook(fifo[2*monitor], pass_arg, makeHeader(0, sizeof(argvs)), bufferSize, sizeof(argvs));
    // write_hook(fifo[2*monitor], dirname, makeHeader(2, strlen(dirname)+1), bufferSize, strlen(dirname)+1);
    write_hook(socketFd[monitor], pass_arg, makeHeader(0, sizeof(argvs)), socketBufferSize, sizeof(argvs));
    write_hook(socketFd[monitor], dirname, makeHeader(2, strlen(dirname)+1), socketBufferSize, strlen(dirname)+1);
    free(dataBuffer);
    free(pass_arg);
}

void distributeSubdirs(parentDataPointer *Universal, int function)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    int numMonitors = *(Universal->numMonitorsPtr); 
    // int *fifo = *(Universal->fifoPtr); 
    int *socketFd = *(Universal->socketPtr);
    Monitor **monitor = *(Universal->monitorPtr);
    char ***sub_dir;
    char *dirname = *(Universal->dirnamePtr);
    if(function==-1)
    {
        DIR *input_dir;
        struct dirent *file;
        input_dir = opendir(dirname);
        if (input_dir == NULL){
            perror("parent opendir");
            exit(EXIT_FAILURE);
        }
        int rr=0;
        char *dataBuffer = malloc_hook(sizeof(struct dirent));
        while((file=readdir(input_dir)) != NULL)
            if(strcmp(file->d_name,".") && strcmp(file->d_name,"..") && file->d_ino!=0)
            {
                //printf("PARENT WRITE %s\n", file->d_name);
                sub_dir = &(monitor[rr]->sub_dir);
                if((*sub_dir) == NULL)
                    (*sub_dir) = malloc_hook(1*(sizeof(char*)));
                else
                    (*sub_dir) = realloc_hook((*sub_dir), (monitor[rr]->Nsub_dirs+1)*(sizeof(char*)));
                (*sub_dir)[monitor[rr]->Nsub_dirs] = malloc_hook(strlen(file->d_name)+1);
                strcpy((*sub_dir)[monitor[rr]->Nsub_dirs],file->d_name);
                // printf("%d %s\n",monitor[rr]->pid, (*sub_dir)[(monitor[rr])->Nsub_dirs]);
                monitor[rr]->Nsub_dirs++;
                // write_hook(fifo[2*rr], file->d_name, makeHeader(2, strlen(file->d_name)+1), bufferSize, strlen(file->d_name)+1);
                // write_hook(socketFd[rr], file->d_name, makeHeader(2, strlen(file->d_name)+1), bufferSize, strlen(file->d_name)+1);
                rr++;
                if(rr>=numMonitors)
                    rr=0;
            }
        free(dataBuffer);
        closedir(input_dir);
    }
    else{
        // for(int i=0; i<monitor[function]->Nsub_dirs;i++)
        //     write_hook(socketFd[function], monitor[function]->sub_dir[i], makeHeader(2, strlen(monitor[function]->sub_dir[i])+1), bufferSize, strlen(monitor[function]->sub_dir[i])+1);
        //     // write_hook(fifo[2*function], monitor[function]->sub_dir[i], makeHeader(2, strlen(monitor[function]->sub_dir[i])+1), bufferSize, strlen(monitor[function]->sub_dir[i])+1);
    }
}

void receiveBloomFilter(parentDataPointer *Universal, int i)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    int bloomSize = *(Universal->bloomSizePtr); 
    // int *fifo = *(Universal->fifoPtr); 
    int *socketFd = *(Universal->socketPtr);
    Monitor ***monitor = Universal->monitorPtr;
    struct monitorBloomFilter **bloomFilter;
    void *receivedBuffer = malloc_hook(1);
    char *temp;
    //(monitor[i]).bloomFilter
    int size = 0;
    //printf("\n\n");
    while(1)
    {
        // if((size=read_hook(fifo[2*i+1],&receivedBuffer,bufferSize))!=0)
        if((size=read_hook(socketFd[i],&receivedBuffer,socketBufferSize))!=0)
        {
            temp = (char*) receivedBuffer;
            //(bloomFilter[i]).virus = malloc_hook(strlen(temp)+1);
            if((*monitor)[i]->bloomFilter == NULL)
                (*monitor)[i]->bloomFilter = (struct monitorBloomFilter**) malloc_hook(sizeof(monitorBloomFilter*));
            else
                (*monitor)[i]->bloomFilter = (struct monitorBloomFilter**) realloc_hook((*monitor)[i]->bloomFilter, sizeof(monitorBloomFilter*)*((*monitor)[i]->NbloomFilters+1));
            bloomFilter = &((*monitor)[i]->bloomFilter[(*monitor)[i]->NbloomFilters]);
            (*bloomFilter) = (struct monitorBloomFilter*) malloc_hook(sizeof(monitorBloomFilter));
            (*bloomFilter)->virus = malloc_hook(size);
            strcpy((*bloomFilter)->virus, temp);
            // printf("%d %s\n",(*monitor)[i]->pid, (*bloomFilter)->virus);
            //printf("[%d] virus received from %d '%s' '%s'\n",cn++,2*i+1,((monitor[i]).bloomFilter[(monitor[i]).NbloomFilters])->virus, temp);
        }
        else break;
        // if(read_hook(fifo[2*i+1],&receivedBuffer,bufferSize)!=0)
        if((size=read_hook(socketFd[i],&receivedBuffer,socketBufferSize))!=0)
        {
            // printf("i read %s bf\n",(*bloomFilter)->virus);
            //printf("{{%d}} bloomFilter received from %d\n",cn1++,2*i+1);
            temp = (char*) receivedBuffer;
            //(*bloomFilter)->bloomFilter = calloc(sizeof(char), bloomSize); 
            (*bloomFilter)->bloomFilter = calloc_hook(sizeof(char),bloomSize);
            //memset((*bloomFilter)->bloomFilter,0,bloomSize);
            //printf("size of temp = %d and bloomsize = %d\n",strlen(receivedBuffer),bloomSize);
            memcpy((*bloomFilter)->bloomFilter, temp, bloomSize);
            //printf("%d %s\n",(monitor[i]).pid, ((monitor[i]).bloomFilter[(monitor[i]).NbloomFilters-1])->virus);
        }
        ((*monitor)[i]->NbloomFilters)++;
        //else break;
    }
    free(receivedBuffer);
}

void updateBloomFilter(parentDataPointer *Universal, int i)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    int bloomSize = *(Universal->bloomSizePtr); 
    // int *fifo = *(Universal->fifoPtr); 
    int *socketFd = *(Universal->socketPtr);
    Monitor ***monitor = Universal->monitorPtr;
    Monitor *myMonitor = (*monitor)[i];
    monitorBloomFilter ***bloomFilter = &(myMonitor->bloomFilter);
    monitorBloomFilter **myBloomFilter;

    char **bF = NULL;
    char **virus = NULL;
    void *receivedBuffer = malloc_hook(1);
    char *temp;
    int counter = 0;
    int size = 0;
    // printf("There are %d bFs\n",(*monitor)[i]->NbloomFilters);
    while(1)
    {
        if(counter>=(*monitor)[i]->NbloomFilters)
            break;
        // printf("Updating %d bF\n",counter);
        // if((size=read_hook(fifo[2*i+1],&receivedBuffer,bufferSize))!=0)
        if((size=read_hook(socketFd[i],&receivedBuffer,socketBufferSize))!=0)
        {
            temp = (char*) receivedBuffer;
            myBloomFilter = &((*bloomFilter)[counter]);
            bF = &((*myBloomFilter)->bloomFilter);
            virus = &((*myBloomFilter)->virus);
            free(*virus);
            free(*bF);
            free(*myBloomFilter);
            (*myBloomFilter) = (struct monitorBloomFilter*) malloc_hook(sizeof(monitorBloomFilter));
            bF = &((*myBloomFilter)->bloomFilter);
            virus = &((*myBloomFilter)->virus);
            (*virus) = malloc_hook(size);
            strcpy((*virus), temp);
            // printf("%d %s\n",myMonitor->pid, (*myBloomFilter)->virus);
        }
        else break;
        // if(read_hook(fifo[2*i+1],&receivedBuffer,bufferSize)!=0)
        if((size=read_hook(socketFd[i],&receivedBuffer,socketBufferSize))!=0)
        {
            temp = (char*) receivedBuffer;
            (*bF) = calloc_hook(sizeof(char),bloomSize);
            memcpy((*bF) , temp, bloomSize);
            strcpy((*bF), temp);
        }
        counter++;
        //else break;
    }
    if(counter!=(*monitor)[i]->NbloomFilters)
        perror("bloomFilters");
    free(receivedBuffer);
}

void sendReadyMsg(parentDataPointer *Universal, int monitor)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    // int *fifo = *(Universal->fifoPtr); 
    char *msg = (char*) malloc_hook(6);
    int *socketFd = *(Universal->socketPtr);
    strcpy(msg,"READY"); 
    //write_hook(fifo[WRITE], msg, makeHeader(2, 6), bufferSize, 6);
    write_hook(socketFd[monitor], msg, makeHeader(2, 6), socketBufferSize, 6);
    free(msg);
    //kill(getppid(),SIGUSR2);
}

int receiveReadyMsg(parentDataPointer *Universal, int monitor)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    // int *fifo = *(Universal->fifoPtr); 
    int *socketFd = *(Universal->socketPtr);
    void *receivedBuffer = malloc_hook(1);
    int success = 0;
    // Monitor* Monitor = *(Universal->monitorPtr);
    //while(1){
    // if(read_hook(fifo[2*monitor+1],&receivedBuffer,bufferSize)!=0)
    if(read_hook(socketFd[monitor],&receivedBuffer,socketBufferSize)!=0)
        if(!strcmp(receivedBuffer, "READY")){
            success++;
            //break;
            //printf("%s\n",receivedBuffer);
        }
        // else
            //printf("%s\n",receivedBuffer);
    //}
    // else{
    //     kill((Monitor[2*monitor+1]).pid, SIGUSR2); 
    //     if(read_hook(fifo[2*monitor+1],&receivedBuffer,bufferSize)!=0)
    //     {
    //         printf("%s\n",receivedBuffer);
    //         if(!strcmp(receivedBuffer, "READY"))
    //         {
    //             success++;
    //             //printf("%s\n",receivedBuffer);
    //         }
    //     }
    // }
    free(receivedBuffer);
    return success;
}

void readInputFromKeyboard(parentDataPointer* Universal)
{
    char *input, **argument;
    input = (char*) malloc(sizeof(char)*101);
    input[0] = '\0';
    // strcpy(input,"\n"); 
    argument = (char**) malloc(sizeof(char*)*6);
    int false = 0;
    if(input==NULL){
        printf("Memory Allocation Error\n");
        exit(1);
    }
    //fflush(stdin);
    //fflush(stdout);
    printf("\n\033[3;39mConnection between the travelMonitorClient and the monitorServers was successfully established.\033[0m\n");
    // printf("\033[3;39mThe application is ready to receive user input from the keyboard.\033[0m\n");
    // printf("\033[2J"); // Clear screen
    while(1)
    {
        printf("\x1B[2K");
        printf("\033[1;31mtravelMonitorClient\033[0m:\033[1;36m~\033[0m$ ");
        free(input);
        free(argument);
        checkFlag(&Universal);  
        input = (char*) malloc(sizeof(char)*101);
        input[0] = '\0';
        argument = (char**) malloc(sizeof(char*)*6);      
        fgets(input,101 , stdin);
        // checkFlag(&Universal);
        if(strlen(input)==1 ){//|| input[0]=='\0' || input[0]=='\n') 
            usleep_hook(100000);
            printf("\033[1A"); // Move up X lines;
            continue;
        }
        else if(input[strlen(input)-1]=='\n') 
            input[strlen(input)-1]='\0';
         
        //divide the line into tokens-words
        argument[0] = strtok(input," ");
        argument[1] = strtok(NULL, " ");
        argument[2] = strtok(NULL, " ");
        argument[3] = strtok(NULL, " ");
        argument[4] = strtok(NULL, " ");
        argument[5] = strtok(NULL, " ");

        //call the right function according to the arguments
        if(!strcmp(argument[0], "/travelRequest"))
            travelRequest(Universal, argument[1], argument[2], argument[3], argument[4], argument[5]);
        else if(!strcmp(argument[0], "/travelStats"))
            travelStats(Universal, argument[1], argument[2], argument[3], argument[4]);
        else if(!strcmp(argument[0], "/addVaccinationRecords"))
            addVaccinationRecords(Universal, argument[1]);
        else if(!strcmp(argument[0], "/searchVaccinationStatus"))
            searchVaccinationStatus(Universal, argument[1]);
        else if(!strcmp(input, "/exit") || !strcmp(input, "quit") || !strcmp(input, "q")){
            exitNow(&Universal,1);
            break;
        }else if(!strcmp(argument[0], "/kill")){
            kill_hook(Universal, argument[1],argument[2]);
        }else if(!strcmp(argument[0], "/help") || !strcmp(argument[0], "h") ){
            printf("valid commands are: \033[0;32m/travelRequest \033[0;0mor\033[0;32m " 
                    "/travelStats \033[0;0mor\033[0;32m /addVaccinationRecords \033[0;0mor\033[0;32m "
                    "/searchVaccinationStatus \033[0;0mor\033[0;32m /exit\n\033[0;0m");
        }else{
            usleep_hook(100000);
            printf("\033[1A"); // Move up X lines;
            continue;
            // printf("\033[31mtravelMonitorClient\033[0;0m: \033[31m%s\033[0;0m: \033[31mcommand not found\033[0;0m\n", input);
        }
    
    }
    free(input);
    free(argument);
}

void travelRequest(parentDataPointer *Universal, char *citizenID, char *date, char *countryFrom, char *countryTo, char *virusName)
{
// /travelRequest char *citizenID, char *date, char *countryFrom, char *countryTo, char *virusName
// H εφαρμογή πρώτα θα ελέγχει το bloom filter που του έχει στείλει το Monitor process που διαχειρίζεται τη
// χώρα countryFrom. Αν το bloom filter υποδεικνύει πως ο πολίτης citizenID δεν έχει εμβολιαστεί
// κατά του virusName τυπώνει
// REQUEST REJECTED – YOU ARE NOT VACCINATED
// Αν το bloom filter υποδεικνύει πως ο πολίτης citizenID ίσως έχει εμβολιαστεί κατά του virusName, η
// εφαρμογη ζητάει μέσω named pipe από το Monitor process που διαχειρίζεται τη χώρα countryFrom αν
// όντως έχει εμβολιαστεί ο citizenID. Το Monitor process απαντάει μέσω named pipe YES/NO όπου στη
// περίπτωση του YES, στέλνει και την ημερομηνία εμβολιασμού. Για την απάντηση το Monitor process μπορεί
// (αλλά δεν είναι απαραίτητο) να χρησιμοποιήσει τις δομές που είχατε στην 1η εργασία (π.χ. τη skip list). Η
// εφαρμογή ελέγχει αν έχει εμβολιαστεί ο πολίτης λιγότερο από 6 μήνες πριν την επιθυμητή ημερομηνία
// ταξιδιού date και τυπώνει ένα από τα ακόλουθα μηνύματα
// REQUEST REJECTED – YOU ARE NOT VACCINATED
// REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE
// REQUEST ACCEPTED – HAPPY TRAVELS
    // struct sigaction *act = Universal->actPtr;
    // act->sa_handler=SIG_IGN; 
    // sigaction (SIGCHLD,act,NULL);
    printf("\033[1A"); // Move up X lines;
    printf("\r\033[1;31mtravelMonitorClient\033[0m:\033[1;36m~/travelRequest/\033[0m$ \033[0m\033[1;39mcitizenID:\033[0m%s \033[1;39mdate:\033[0m%s \033[1;39mcountryFrom:\033[0m%s \033[1;39mcountryTo:\033[0m%s \033[1;39mvirusName:\033[0m%s\n", citizenID, date, countryFrom, countryTo, virusName);
    int manager = -1;
    int bloomSize = *(Universal->bloomSizePtr);
    int socketBufferSize = *(Universal->socketBufferSizePtr);
    // int *fifo = *(Universal->fifoPtr);
    int *socketFd = *(Universal->socketPtr);
    Monitor **monitor = *(Universal->monitorPtr);
    char *bloomFilter = NULL;
    char *bloomString = NULL;
    char *request = NULL;
    void *receivedBuffer = NULL;
    char **retval = NULL;
    int size=0;
    bloomFilter = findBloomFilter(Universal, countryFrom, virusName, &manager);
    if(bloomFilter==NULL || manager==-1){
        printf("There is no such bloomFilter\n");
        return;
    } 
    // else printf("manager = %d\n", manager);
    bloomString = (char*) malloc_hook(sizeof(char)*(strlen(citizenID)+strlen(virusName)+1));
    snprintf(bloomString, strlen(citizenID)+strlen(virusName)+1, "%s%s", citizenID, virusName);
    if(!isVaccinated_bloomFilter(bloomFilter, bloomSize, (unsigned char *) bloomString))
    {
        printf(RED"REQUEST REJECTED – YOU ARE NOT VACCINATED\n"RESET);
        addRequest(Universal, date, countryTo, NULL, 0, 0);
    }
    else
    {
        // printf("%s is possibly vaccinated to %s\n", citizenID, virusName);
        //write request
        request = (char*) malloc_hook(sizeof(char)*(8+strlen(citizenID)+strlen(virusName)+strlen(date)+4));
        snprintf(request, 8+strlen(citizenID)+strlen(virusName)+strlen(date)+4, "REQUEST|%s|%s|%s|", citizenID, virusName,date); 
        //wake up monitor 
        kill(monitor[manager]->pid, SIGCONT);
        //send request
        // write_hook(fifo[2*manager], request, makeHeader(2, strlen(request)+1), bufferSize, strlen(request)+1);
        write_hook(socketFd[manager], request, makeHeader(2, strlen(request)+1), socketBufferSize, strlen(request)+1);
        usleep_hook(100);
        // if((size=read_hook(fifo[2*manager+1],&receivedBuffer,bufferSize))!=0)
        if((size=read_hook(socketFd[manager],&receivedBuffer,socketBufferSize))!=0)
        {
            retval = malloc_hook(sizeof(char*)*2);
            retval[0] = strtok((char*) receivedBuffer,"|");
            retval[1] = strtok(NULL, "|");
            if(!strcmp(retval[0],"YES"))
            {
                // printf("vaccinated at %s with tDate at %s\n",retval[1],date);
                if(recentlyVacced(retval[1], date)>0){
                    printf(GREEN"REQUEST ACCEPTED – HAPPY TRAVELS\n"RESET);
                    addRequest(Universal, date, countryTo, NULL, 1, 0);
                }
                else{
                    printf(YELLOW"REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n"RESET);
                    addRequest(Universal, date, countryTo, NULL, 0, 0);
                }
            }
            else if(!strcmp(retval[0],"NO")){
                printf(RED"REQUEST REJECTED – YOU ARE NOT VACCINATED\n"RESET);
                addRequest(Universal, date, countryTo, NULL, 0, 0);
            }
        }
        free(retval);
        free(receivedBuffer);
    }
    free(request);
    free(bloomString);

    // act = Universal->actPtr;
    // act->sa_handler=parentInterrupt; 
    // sigaction (SIGCHLD,act,NULL);
}

void travelStats(parentDataPointer *Universal, char *virusName, char *date1, char *date2, char *countryTo)
{
// travelStats virusName date1 date2 [country]
// Αν δεν δοθεί country όρισμα, η εφαρμογή θα τυπώνει τον αριθμό πολιτών που έχουν ζητήσει έγκριση να
// ταξιδέψουν μεσα στο διαστημα [date1...date2] σε χώρες που ελέγχουν για εμβολιασμό κατά του
// virusName, και τον αριθμο πολιτών που εγκρίθηκαν και που απορρίφθηκαν. Aν δοθεί country
// όρισμα, η εφαρμογή θα τυπώνει την ίδια πληροφορία αλλά μόνο για τη συγκεκριμένη χώρα country. Τα
// date1 date2 ορίσματα θα έχουν μορφή DD-MM-YYYY.
// Output format: Παράδειγμα:
// TOTAL REQUESTS 29150
// ACCEPTED 25663
// REJECTED 3487
    printf("\033[1A"); // Move up X lines;
    printf("\r\033[1;31mtravelMonitorClient\033[0m:\033[1;36m~/travelStats/\033[0m$ \033[0m\033[1;39mvirusName:\033[0m%s \033[1;39mdate1:\033[0m%s \033[1;39mdate2:\033[0m%s \033[1;39mcountryTo:\033[0m%s\n", virusName, date1, date2, (countryTo!=NULL)?countryTo:" ---");
    getRequests(Universal, date1, date2, countryTo, virusName, 0);  
}

void addVaccinationRecords(parentDataPointer *Universal, char *country)
{
// Με αυτό το αίτημα ο χρήστης έχει τοποθετήσει στο input_dir/country ένα ή περισσότερα αρχεία για
// επεξεργασία από το Monitor process. Το parent process στέλνει ειδοποίηση μέσω SIGUSR1 σήμα στο
// Monitor process που διαχειρίζεται τη χώρα country ότι υπάρχουν input files για ανάγνωση στον
// κατάλογο. Το Monitor process διαβάζει ότι νέο αρχείο βρει, ενημερώνει τις δομές δεδομένων και στέλνει
// πίσω στο parent process, μέσω named pipe, τα ενημερωμένα του bloom filters που αναπαριστούν το
// σύνολο πολιτών που έχουν εμβολιαστεί
    // struct sigaction *act = Universal->actPtr;
    // act->sa_handler=SIG_IGN; 
    // sigaction (SIGCHLD,act,NULL);
    printf("\033[1A"); // Move up X lines;
    printf("\r\033[1;31mtravelMonitorClient\033[0m:\033[1;36m~/addVaccinationRecords/\033[0m$ \033[1;39mcountry:\033[0m%s\n", (country!=NULL)?country:" ---");
    
    int manager = -1;
    Monitor **monitor = *(Universal->monitorPtr);
    int *socketFd = *(Universal->socketPtr);
    int socketBufferSize = *(Universal->socketBufferSizePtr);
    char *command = NULL;
    findBloomFilter(Universal, country, NULL, &manager);
    if(manager==-1){
        printf("ERROR: no %s country\n",country);
        return;
    }
    kill(monitor[manager]->pid, SIGCONT);
    //kill(monitor[manager]->pid, SIGUSR1);

    command = (char*) malloc_hook(sizeof(char)*12);
    snprintf(command, 12, "ADDRECORDS|"); 
    write_hook(socketFd[manager], command, makeHeader(2, strlen(command)+1), socketBufferSize, strlen(command)+1);
    usleep_hook(150000);            //wait for monitor to do what he has got to do
    updateBloomFilter(Universal, manager);
    // kill(monitor[manager]->pid,SIGCONT);
    while(1)
        if(receiveReadyMsg(Universal, manager)){
            printf("Monitor %d is ready\n",monitor[manager]->pid);
            break;
        }
    free(command);
    // kill(monitor[manager]->pid,SIGCONT);
    // act = Universal->actPtr;
    // act->sa_handler=parentInterrupt; 
    // sigaction (SIGCHLD,act,NULL);
}

void searchVaccinationStatus(parentDataPointer *Universal, char *citizenID)
{
// To parent process προωθεί σε όλους τα Monitor processes το αίτημα μέσω named pipes. Το Monitor process
// που διαχειρίζεται τον πολίτη με αναγνωριστικό citizenID στέλνει μέσω named pipe ό,τι πληροφορίες
// έχει για τους εμβολιασμούς που έχει κάνει/δεν έχει κάνει ο συγκεκριμένος πολίτης. Όταν λάβει τις
// πληροφορίες, το parent τις τυπώνει στο stdout.
// Output format: Παράδειγμα:
// 889 JOHN PAPADOPOULOS GREECE
// AGE 52
// COVID-19 VACCINATED ON 27-12-2020
// H1N1 NOT YET VACCINATED
    // struct sigaction *act = Universal->actPtr;
    // act->sa_handler=SIG_IGN; 
    // sigaction (SIGCHLD,act,NULL);
    printf("\033[1A"); // Move up X lines;
    printf("\r\033[1;31mtravelMonitorClient\033[0m:\033[1;36m~/searchVaccinationStatus/\033[0m$ \033[1;39mcitizenID:\033[0m%s\n", (citizenID!=NULL)?citizenID:" --- ");
    int socketBufferSize = *(Universal->socketBufferSizePtr);
    // int *fifo = *(Universal->fifoPtr);
    int *socketFd = *(Universal->socketPtr);
    int numMonitors = *(Universal->numMonitorsPtr);
    Monitor **monitor = *(Universal->monitorPtr);
    char *search = NULL;
    void *receivedBuffer = NULL;
    int size=0;  
    for(int manager=0; manager<numMonitors; manager++)
    {
        //write request
        search = (char*) malloc_hook(sizeof(char)*(strlen(citizenID)+9));
        snprintf(search, strlen(citizenID)+9, "SEARCH|%s|", citizenID);
        //wake up monitor
        kill(monitor[manager]->pid, SIGCONT);
        //send request
        // write_hook(fifo[2*manager], search, makeHeader(2, strlen(search)+1), bufferSize, strlen(search)+1);
        write_hook(socketFd[manager], search, makeHeader(2, strlen(search)+1), socketBufferSize, strlen(search)+1);
        free(search); 
        usleep_hook(100);
        // receivedBuffer = malloc_hook(1);
        receivedBuffer = NULL;
        // while((size=read_hook(fifo[2*manager+1],&receivedBuffer,bufferSize))>0)
        while((size=read_hook(socketFd[manager],&receivedBuffer,socketBufferSize))>0)
        {
            printf("%s\n", (char*) receivedBuffer);
            free_hook(receivedBuffer);                        
            receivedBuffer=NULL;
        }
        if(size<=0){            //controversial, it might cause error
            free_hook(receivedBuffer);                        
            receivedBuffer=NULL;
        }
    }
    // act = Universal->actPtr;
    // act->sa_handler=parentInterrupt; 
    // sigaction (SIGCHLD,act,NULL);
}

void kill_hook(parentDataPointer *Universal, char *arg, char *arg2)
{
    Monitor **monitor = *(Universal->monitorPtr);
    int numMonitors = *(Universal->numMonitorsPtr);
    int i=-1;
    if(strcmp(arg,"me") && strcmp(arg,"all"))
    {
        i=atoi(arg);
        if(i<0 || i>numMonitors-1)
        {
            
            printf("There is no %d monitor\n",i);
            return;
        }
        printf("\033[1A"); // Move up X lines;
        printf("\r\033[1;31mtravelMonitorClient\033[0m:\033[1;36m~/kill/\033[0m$ \033[1;39msignal:\033[0m%s \033[1;39m%s:\033[0m%d\033[0m\n", arg2, "proccess", monitor[i]->pid);
    }
    else if(!strcmp(arg,"me") || !strcmp(arg,"all")){
        printf("\033[1A"); // Move up X lines;
        printf("\r\033[1;31mtravelMonitorClient\033[0m:\033[1;36m~/kill/\033[0m$ \033[1;39msignal:\033[0m%s \033[1;39m%s:\033[0m%s\033[0m\n", arg2, strcmp(arg,"all")?"proccess":"proccesses" ,arg);
    }

    if(!strcmp(arg2,"SIGINT")){
        if(i==-1){
            if(!strcmp(arg,"me"))
                kill(getpid(), SIGINT);
            else if(!strcmp(arg,"all"))
                for(int k=0; k<numMonitors;k++)
                    kill(monitor[k]->pid, SIGINT);
        }
        else 
            kill(monitor[i]->pid, SIGINT);
    }
    else if(!strcmp(arg2,"SIGQUIT")){
        if(i==-1){
            if(!strcmp(arg,"me"))
                kill(getpid(), SIGQUIT);
            else if(!strcmp(arg,"all"))
                for(int k=0; k<numMonitors;k++)
                    kill(monitor[k]->pid, SIGQUIT);
        }
        else 
            kill(monitor[i]->pid, SIGQUIT);
    }
    else if(!strcmp(arg2,"SIGKILL")){
        if(i==-1){
            if(!strcmp(arg,"me"))
                kill(getpid(), SIGKILL);
            else if(!strcmp(arg,"all"))
                for(int k=0; k<numMonitors;k++)
                    kill(monitor[k]->pid, SIGKILL);
        }
        else 
            kill(monitor[i]->pid, SIGKILL);
    }
    else if(!strcmp(arg2,"SIGUSR1")){
        if(i==-1){
            if(!strcmp(arg,"me"))
                kill(getpid(), SIGUSR1);
            else if(!strcmp(arg,"all"))
                for(int k=0; k<numMonitors;k++)
                    kill(monitor[k]->pid, SIGUSR1);
        }
        else 
            kill(monitor[i]->pid, SIGUSR1);
    }
}

void exitNow(parentDataPointer **Universal, int function)
{
    if(function==1){
        printf("\033[1A"); // Move up X lines;
        printf("\r\033[1;31mtravelMonitorClient\033[0m:\033[1;36m~/exit\033[0m$ \033[0m\n");
    }
    parentDataPointer *temp = *Universal;
    // int *fifo = *(temp->fifoPtr);
    int *socketFd = *(temp->socketPtr);
    int socketBufferSize = *(temp->socketBufferSizePtr);
    int numMonitors = *(temp->numMonitorsPtr);
    Monitor **monitor = *(temp->monitorPtr);
    struct sigaction act = *(temp->actPtr);
    char **argument = *(temp->argumentPtr);
    free(argument[0]);
    free(argument);
    
    mkdir(LOG_DIR, 0777);
    int file;
    int pid = getpid();
    char *filename = malloc_hook(countDigits(pid)+10+strlen(LOG_DIR));
    snprintf(filename, countDigits(pid)+10+strlen(LOG_DIR) , LOG_DIR"log_file.%d", pid);
    if((file = creat(filename, 0777))<0) 
    {
        perror("creat");
        exit(1); 
    }     
    int fd = dup(1);
    if(dup2(file,1)<0) 
    {
        perror("dup2"); 
        exit(1);
    }

    for(int i=0;i<numMonitors;i++)
        for(int j=0;j<monitor[i]->Nsub_dirs;j++)
            printf("%s\n",monitor[i]->sub_dir[j]);
    getRequests(temp, NULL, NULL, NULL, NULL, 0);
    dup2(fd, 1);
    close(fd);
    free(filename);     

    reqReg *requestsRegistry = *(temp->requestsRegistryPtr);
    requests *Requests;
    if(requestsRegistry!=NULL){
        for(int i=0;i<requestsRegistry->n;i++){
            Requests = requestsRegistry->array[i];
            free(Requests->countryTo);
            for(int j=0;j<Requests->nt;j++)
                free(Requests->totalTravelRequestsDate[j]);
            free(Requests->totalTravelRequestsDate);
            free(Requests->totalTravelRequestsCtr);
            for(int j=0;j<Requests->na;j++)
                free(Requests->acceptedRequestsDate[j]);
            free(Requests->acceptedRequestsDate);
            free(Requests->acceptedRequestsCtr);
            for(int j=0;j<Requests->nr;j++)
                free(Requests->rejectedRequestsDate[j]);
            free(Requests->rejectedRequestsDate);
            free(Requests->rejectedRequestsCtr);
            free(requestsRegistry->array[i]);
        }
        free(requestsRegistry->array);
        free(requestsRegistry);
    }   
    char *dirname = *(temp->dirnamePtr);
    // char **namedPipe = *(temp->namedPipePtr);
    free(dirname);
    ////Waiting for processes to exit normally and receive SIGCHILD
    // act.sa_handler=SIG_IGN; 
    // sigaction (SIGCHLD,&act,NULL);
    // for(int i=0;i<numMonitors;i++) 
    //     wait(NULL);
    // if(function)
    // {
    act.sa_handler=SIG_IGN; 
    sigaction (SIGCHLD,&act,NULL);
    //write exit notification
    char *exitNow = malloc_hook(5);
    snprintf(exitNow, 5, "EXIT"); 
    for(int i=0;i<numMonitors;i++){
        //wake up monitor
        kill(monitor[i]->pid, SIGCONT); 
        //send notification
        write_hook(socketFd[i], exitNow, makeHeader(2, strlen(exitNow)+1), socketBufferSize, strlen(exitNow)+1);
    }
    free(exitNow);
    for(int i=0;i<numMonitors;i++)
        wait(NULL);
    // }
    // act.sa_handler=SIG_IGN; 
    // sigaction (SIGCHLD,&act,NULL);
    // for(int i=0;i<numMonitors;i++){
    //     kill(monitor[i]->pid, SIGKILL); 
    // }
    // for(int i=0;i<numMonitors;i++){
    //     wait(NULL);
    //     //printf("Monitor %d was killed\n",monitor[i]->pid);
    // }

    monitorBloomFilter *bF;
    for(int i=0;i<numMonitors;i++){
        for(int j=0;j<monitor[i]->Nsub_dirs;j++){
            free_hook(monitor[i]->sub_dir[j]);
        }
        free_hook(monitor[i]->sub_dir);
        // if(function)
        // {
            for(int j=0;j<monitor[i]->NbloomFilters;j++){
                free_hook(monitor[i]->bloomFilter[j]->virus);
                free_hook(monitor[i]->bloomFilter[j]->bloomFilter);
            }
            for(int j=0;j<monitor[i]->NbloomFilters;j++){
                bF = monitor[i]->bloomFilter[j];
                free_hook(bF);
            }
        // }
        free_hook(monitor[i]->bloomFilter);
        free_hook(monitor[i]);
    }
    // free(monitor);   //shows free error
    free(*(temp->monitorPtr));
    // for(int i=0;i<numMonitors*2;i++) 
    //     close(fifo[i]);
    // free(fifo);
    // for(int i=0;i<numMonitors*2;i++){
    //     unlink(namedPipe[i]);
    //     free(namedPipe[i]);
    // }
    for(int i=0;i<numMonitors;i++){
        if(shutdown(socketFd[i], SHUT_RDWR)<0)
            perror("shudown socket");
        if(close(socketFd[i])<0)
            perror("close socket");   
    }
    free(socketFd);
    // free(namedPipe);
    //free the Universal Datapointer 
    free(*Universal);
    setlocale(LC_ALL, "");
    printf("%ls", L"\033[1;37mΕυχαριστούμε που χρησιμοποιήσατε την εφαρμογή travelMonitorClient.\033[0m\n");
    if(!function){
        rmdir("../tmp/namedPipes");
        rmdir("../tmp");
        exit(EXIT_SUCCESS);
    }
}
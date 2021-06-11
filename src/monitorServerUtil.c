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
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>

#define LOG_DIR "../log_folder/"

#ifndef monitorServerUtilHeader
#define monitorServerUtilHeader
#include "../lib/headers/monitorServerUtil.h"
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
#include "../lib/headers/monitorSignals.h"
#endif

#ifndef circularBufferHeader
#define circularBufferHeader
#include "../lib/headers/circularBuffer.h"
#endif

#define READ 0
#define WRITE 1

extern int FLAGUSR1;
extern int FLAGUSR2;

pthread_mutex_t cyclicMutex;
pthread_cond_t cyclicReadCond;
pthread_cond_t cyclicWriteCond;
int exitThread = 0;

void *threadParse(void *arg)
{
    monitorDataPointer *Universal = (monitorDataPointer *) arg;
    cyclicBuffer *cyclicbuffer = *(Universal->cyclicbufferPtr);
    char *readval = NULL;
    while(1)
    {    
        pthread_mutex_lock(&cyclicMutex);
        while(!isReadable_circularBuffer(&cyclicbuffer)){
            // printf("son got blocked\n");
            pthread_cond_wait(&cyclicReadCond,&cyclicMutex);
            if(exitThread){
                pthread_mutex_unlock(&cyclicMutex);
                return NULL;
                // pthread_exit(NULL);
            } 
        }
        readval = readFrom_circularBuffer(&cyclicbuffer);
        isFileRead(Universal->mySubDirPtr, readval);
        parse(Universal, readval);
        storeDataIntoDatatypes(Universal);
        pthread_mutex_unlock(&cyclicMutex);
        pthread_cond_signal(&cyclicWriteCond);
    }
    // printf("i exited mutex area %d\n",pthread_self());
}

void *Main(void *arg)
{
    monitorDataPointer **UniversalPtr = (monitorDataPointer **) arg;
    monitorDataPointer *Universal = (*UniversalPtr);
    char **argv = *(Universal->argvPtr);
    int mySocket = *(Universal->mySocketPtr);
    int socketBufferSize = *(Universal->socketBufferSizePtr);
    int myPort = *(Universal->myPortPtr);
    char **dirname = Universal->dirnamePtr;
    int numThreads = *(Universal->numThreadsPtr);

    // cyclicBuffer *cyclicbuffer = (cyclicBuffer*) malloc_hook(sizeof(cyclicBuffer));
    // create_circularBuffer(&cyclicbuffer, *(Universal->cyclicBufferSizePtr));
    // (*UniversalPtr)->cyclicbufferPtr = &cyclicbuffer;
    cyclicBuffer *cyclicbuffer = *(Universal->cyclicbufferPtr);

    // pthread_t *parser = (pthread_t*) malloc_hook(sizeof(pthread_t)*numThreads);
    // (*UniversalPtr)->parserPtr = &parser;
    pthread_t *parser = *(Universal->parserPtr);

    pthread_mutex_init(&cyclicMutex, NULL);
    pthread_cond_init(&cyclicReadCond, NULL);
    pthread_cond_init(&cyclicWriteCond, NULL);

    for(int i=0;i<numThreads;i++)
        if(pthread_create(&parser[i], NULL, &threadParse, Universal)!=0){
            printf("error: pthread_create\n");
            exit(EXIT_FAILURE);
        }

    struct sockaddr_in server_address;
    if ((mySocket = socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        printf("error: socket\n");
        exit(EXIT_FAILURE);
    }

    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(myPort);
    
    struct hostent *host_entry;
    char hostname[HOST_NAME_MAX+1];
    char *myIPaddress;
    if(gethostname(hostname, HOST_NAME_MAX)==-1)
    {
        printf("error: gethostname");
        exit(EXIT_FAILURE);
    }
    if((host_entry = gethostbyname(hostname))==NULL)
    {
        printf("error: gethostbyname");
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
        printf("error: non-valid IP address\n");
        exit(EXIT_FAILURE);
    }

    if(inet_pton(AF_INET, myIPaddress, &server_address.sin_addr)<=0) 
    {
        printf("error: inet_pton: non-valid IP address\n");
        exit(EXIT_FAILURE);
    }

    if (connect(mySocket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("error: connect\n");
        exit(EXIT_FAILURE);
    }

    *(Universal->mySocketPtr) = mySocket;

    void *receivedBuffer = malloc_hook(1);
    if(read_hook(mySocket,&receivedBuffer,1)!=0){
        (*dirname) = (char*) malloc_hook(strlen(receivedBuffer)+1);            
        strcpy((*dirname), receivedBuffer);
    }
    else
        printf("error: did not receive input_dir from parent\n");
    free(receivedBuffer);

    //parse the subdirectories assigned to you
    receiveSubdirs(Universal,argv);
    monitorDir *mySubDir = *(Universal->mySubDirPtr); 

    printf("\n     \x1B[7mmonitorServer %d\x1B[0m          \x1B[1m---->\x1B[0m     \x1B[7mIP address:\x1B[0m \x1B[1m%s\x1B[0m       ",getpid(),myIPaddress);
    printf("\x1B[7mPort :\x1B[0m \x1B[1m%d\x1B[0m          ", *(Universal->myPortPtr));
    printf("\x1B[7mCountries :\x1B[0m");
    for(int i=0;i<mySubDir->Nsub_dirs;i++)
        if(!i)
            printf(" \x1B[1m%s\x1B[0m ",mySubDir->sub_dir[i]);
        else
            printf("\x1B[1m& %s\x1B[0m ",mySubDir->sub_dir[i]);
    printf("\n");


    counter = 0;
    while(1)
    {
        if(counter>=mySubDir->Nfiles)
            break;
        // printf("%d/%d: %s\n",counter,mySubDir->Nfiles,mySubDir->file[counter]);
        pthread_mutex_lock(&cyclicMutex);
        if(isWritable_circularBuffer(&cyclicbuffer)){
            // printf("parent wrote %s\n",mySubDir->file[counter]);
            writeAt_circularBuffer(&cyclicbuffer, mySubDir->file[counter]);
            counter++;
        }
        else{  //wait till buffer has space for you to write on
            // printf("par thread got blocked\n");
            pthread_cond_wait(&cyclicWriteCond,&cyclicMutex);
        }
        pthread_mutex_unlock(&cyclicMutex);
        pthread_cond_signal(&cyclicReadCond);
    }
    
    //wait for the parser threads to do their job 
    while(1)    
    {
        pthread_mutex_lock(&cyclicMutex);
        while(isReadable_circularBuffer(&cyclicbuffer)){
            pthread_cond_wait(&cyclicWriteCond,&cyclicMutex);
        }
        pthread_mutex_unlock(&cyclicMutex);
        break;
    }

    //send the bloomFilters
    sendBloomFilters(Universal);

    //receive READY message
    while(!receiveReadyMsg(Universal));
    
    //send READY message
    sendReadyMsg(Universal);     

    //wait for commands from parent
    waitForParent(Universal);

    return NULL;
    //pthread_exit(NULL);
}

void uploadMonitorData(monitorDataPointer **Universal, Record ***Registry, Sentinel ***skipList, char ***bloomFilter, char ***countryData,
                       char ***virusData, char ***vacced, char **dirname, int *socketBufferSize, int *cyclicBufferSize, int *NbloomFilters, int *NskipLists, 
                       int *bloomSize, int *Ncountries, int *inputSize, monitorDir **mySubDir,
                       int **fifo, int *mySocket, int *myPort, int *numThreads, reqReg **requestsRegistry, char ***argv,
                        cyclicBuffer **cyclicbuffer, pthread_t **parser)
{
    (*Universal) = (monitorDataPointer*) malloc(sizeof(monitorDataPointer));
    (*Universal)->RegistryPtr = Registry;
    (*Universal)->skipListPtr = skipList;
    (*Universal)->bloomFilterPtr = bloomFilter;
    (*Universal)->countryDataPtr = countryData;
    (*Universal)->virusDataPtr = virusData;
    (*Universal)->vaccedPtr = vacced;
    (*Universal)->dirnamePtr = dirname;
    (*Universal)->socketBufferSizePtr = socketBufferSize;
    (*Universal)->cyclicBufferSizePtr = cyclicBufferSize;
    (*Universal)->NbloomFiltersPtr = NbloomFilters;
    (*Universal)->NskipListsPtr = NskipLists;
    (*Universal)->bloomSizePtr = bloomSize;
    (*Universal)->NcountriesPtr = Ncountries;
    (*Universal)->inputSizePtr = inputSize;
    (*Universal)->mySubDirPtr = mySubDir;
    (*Universal)->requestsRegistryPtr = requestsRegistry;
    (*Universal)->fifoPtr = fifo;
    (*Universal)->mySocketPtr = mySocket;
    (*Universal)->myPortPtr = myPort;
    (*Universal)->argvPtr = argv;
    (*Universal)->numThreadsPtr = numThreads;
    (*Universal)->initSL = 0;
    (*Universal)->cyclicbufferPtr = cyclicbuffer;
    (*Universal)->parserPtr = parser;

    //allocate and nullify the hash table array (pointers to linkedLists)
    (*Registry) = (Record**) malloc(10 * sizeof(Record*));
    for(int i=0;i<10;i++)
		  (*Registry)[i] = NULL;
    //allocated the vaccinated array which stores the values "YES" or "NO" to avoid information reoccurence
    //the citizen records' vaccinated field will point towards this array
    (*vacced) = (char**) malloc(sizeof(char*)*2);
    (*vacced)[0] = (char*) malloc(sizeof(char)*3);
    strcpy((*vacced)[0], "NO");
    (*vacced)[1] = (char*) malloc(sizeof(char)*4);
    strcpy((*vacced)[1], "YES");

    if(mySubDir==NULL)  return;
    (*mySubDir) = (monitorDir*) malloc_hook(sizeof(monitorDir));
    (*mySubDir)->Nsub_dirs = 0;
    (*mySubDir)->Nread_files = 0;
    (*mySubDir)->Nfiles = 0;
    (*mySubDir)->sub_dir = NULL;
    (*mySubDir)->read_files = NULL;
    (*mySubDir)->file = NULL;

    (*requestsRegistry) = malloc_hook(sizeof(reqReg));
    (*requestsRegistry)->n = 0;
    (*requestsRegistry)->array = NULL;

    // (*fifo) = (int*) malloc_hook(sizeof(int)*2);
    // (*fifo)[READ] = open(argv[1], O_RDONLY);
    // if((*fifo)[READ]<0) 
    //     perror("fifo");
    // (*fifo)[WRITE] = open(argv[2], O_WRONLY);
    // if((*fifo)[WRITE]<0) 
    //     perror("fifo");
}

void parametersCheck(monitorDataPointer* Universal, int argc, char **argv)
{
    int *port = Universal->myPortPtr;
    int *numThreads = Universal->numThreadsPtr;
    int *socketBufferSize = Universal->socketBufferSizePtr;
    int *cyclicBufferSize = Universal->cyclicBufferSizePtr;
    int *bloomSize = Universal->bloomSizePtr;
    int counter=0;

    if(argc<12){
        printf("Wrong number of parameters %d\n./monitorServer -p port -t numThreads -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom path1 path2 … pathn\n",argc);
        while(counter<argc)
            printf("%s ",argv[counter++]);
        printf("\n");
        exit(1);
    }   

    if(!strcmp("-p", argv[1]))
        (*port)=atoi(argv[2]);
    else if(!strcmp("-p", argv[3]))
        (*port)=atoi(argv[4]);
    else if(!strcmp("-p", argv[5]))
        (*port)=atoi(argv[6]);
    else if(!strcmp("-p", argv[7]))
        (*port)=atoi(argv[8]);
    else if(!strcmp("-p", argv[9]))
        (*port)=atoi(argv[10]);
    else if(!strcmp("-p", argv[11]))
        (*port)=atoi(argv[12]);
    else{
        printf("Wrong parameters\n./monitorServer -p port -t numThreads -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom path1 path2 … pathn\n");
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
        printf("Wrong parameters\n./monitorServer -p port -t numThreads -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom path1 path2 … pathn\n");
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
        printf("Wrong parameters\n./monitorServer -p port -t numThreads -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom path1 path2 … pathn\n");
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
        printf("Wrong parameters\n./monitorServer -p port -t numThreads -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom path1 path2 … pathn\n");
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
        printf("Wrong parameters\n./monitorServer -p port -t numThreads -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom path1 path2 … pathn\n");
        exit(1);
    }

    srand(time(NULL)); //initialize pseudo-random number generator for coinFlip function that will be used in the skipList creation

    cyclicBuffer **cyclicbuffer = Universal->cyclicbufferPtr;
    pthread_t **parser = Universal->parserPtr;
    // (*cyclicbuffer) = (cyclicBuffer*) malloc_hook(sizeof(cyclicBuffer));
    create_circularBuffer(cyclicbuffer, (*cyclicBufferSize));
    (*parser) = (pthread_t*) malloc_hook(sizeof(pthread_t)*(*numThreads));
}

void receiveInfo(monitorDataPointer *Universal)
{
    int *socketBufferSize = Universal->socketBufferSizePtr; 
    char **dirname = Universal->dirnamePtr;
    int *bloomSize = Universal->bloomSizePtr; 
    // int *fifo = *(Universal->fifoPtr); 
    void *receivedBuffer = malloc_hook(1);
    argvs *pass_arg;
    int mySocket = *(Universal->mySocketPtr);
    // printf("mysock %d\n",mySocket);
    //sleep(2);
    // if(read_hook(fifo[READ],&receivedBuffer,1)!=0){
    if(read_hook(mySocket,&receivedBuffer,1)!=0){
        pass_arg = (argvs*) receivedBuffer;             
        (*socketBufferSize) = pass_arg->bufferSize;  
        (*bloomSize) = pass_arg->bloomSize;  
        printf("received bloomsize=%d and buffersize=%d\n",(*bloomSize),(*socketBufferSize));  
    }
    else
        printf("received nothingg\n");
    // if(read_hook(fifo[READ],&receivedBuffer,1)!=0){
    if(read_hook(mySocket,&receivedBuffer,1)!=0){
        (*dirname) = (char*) malloc_hook(strlen(receivedBuffer)+1);            
        strcpy((*dirname), receivedBuffer);
        printf("received %s\n",(*dirname)); 
    }
    else
        printf("received nothingg\n");
    free(receivedBuffer);
}

void receiveSubdirs(monitorDataPointer *Universal, char **argv)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    // int *fifo = *(Universal->fifoPtr); 
    int mySocket = *(Universal->mySocketPtr);
    monitorDir *mySubDir = *(Universal->mySubDirPtr); 
    char *Dir;
    // void *receivedBuffer = malloc_hook(1);
    DIR *passed_dir;
    struct dirent *passed_file;
    char *filename = malloc(1);
    struct stat filestatus; 
    char *dirname = *(Universal->dirnamePtr);    
    char *temp = NULL;
    int found = 0;
    //while(read_hook(fifo[READ],&receivedBuffer,bufferSize)!=NULL){
    //while(read_hook(mySocket,&receivedBuffer,bufferSize)!=0){
    int counter = 0;
    while(1){
        if(argv[11+counter]==NULL)
            break;
        Dir = argv[11+counter];
        counter++;
        // printf("%s\n",Dir);
        if(!mySubDir->Nsub_dirs){
            mySubDir->sub_dir = (char**) malloc_hook(sizeof(char*)*1);
            mySubDir->sub_dir[0] = (char*) malloc_hook(strlen(Dir)+1);
            strcpy(mySubDir->sub_dir[0], Dir);
            mySubDir->Nsub_dirs++;
        }
        else{
            for(int i=0;i<mySubDir->Nsub_dirs;i++)
                if(!strcmp(mySubDir->sub_dir[i],Dir))
                    found = 1;
            if(!found){
                mySubDir->sub_dir = (char**) realloc_hook(mySubDir->sub_dir, sizeof(char*)*((long unsigned int) (mySubDir->Nsub_dirs+1)));
                mySubDir->sub_dir[mySubDir->Nsub_dirs] = (char*) malloc_hook(strlen(Dir)+1);
                strcpy(mySubDir->sub_dir[mySubDir->Nsub_dirs], Dir);
                mySubDir->Nsub_dirs++;
            }
            found = 0;
        }

        temp = (char*) malloc(sizeof(char)*(strlen(dirname)+strlen(Dir)+2));
        // printf("file %s\n",file);
        snprintf(temp, strlen(dirname)+strlen(Dir)+2, "%s/%s", dirname, Dir);
        // printf("temp %s\n",temp);
        passed_dir = opendir(temp);
        if (passed_dir == NULL){
            perror("opendir");
            exit(EXIT_FAILURE);
        }
        while((passed_file=readdir(passed_dir)) != NULL){
            if(strcmp(passed_file->d_name,".") && strcmp(passed_file->d_name,"..") && passed_file->d_ino!=0){
                filename = (char*) realloc(filename, strlen(dirname)+2*strlen(Dir)+10); 
                snprintf(filename, strlen(temp)+strlen(passed_file->d_name)+2 , "%s/%s", temp, passed_file->d_name);
                if(!isFileRead(Universal->mySubDirPtr, passed_file->d_name))
                {
                    // printf("%s\n",passed_file->d_name);
                    // mySubDir = *(Universal->mySubDirPtr); 
                    if (stat(filename, &filestatus) == -1){
                        perror("stat");
                        exit(EXIT_FAILURE);
                    }
                    else{
                        if(filestatus.st_size>1){
                            if(!mySubDir->Nfiles){
                                // printf("%s\n",filename);
                                mySubDir->file = (char**) malloc_hook(sizeof(char*)*1);
                                mySubDir->file[0] = (char*) malloc_hook(strlen(filename)+1);
                                strcpy(mySubDir->file[0], filename);
                                mySubDir->Nfiles=1;
                            }
                            else{
                                for(int i=0;i<mySubDir->Nfiles;i++)
                                    if(!strcmp(mySubDir->file[i],filename)){
                                        found = 1;
                                        // printf("found\n");
                                    }
                                if(!found){
                                    // printf("%s\n",filename);
                                    mySubDir->file = (char**) realloc_hook(mySubDir->file, sizeof(char*)*((long unsigned int) (mySubDir->Nfiles+1)));
                                    mySubDir->file[mySubDir->Nfiles] = (char*) malloc_hook(strlen(filename)+1);
                                    strcpy(mySubDir->file[mySubDir->Nfiles], filename);
                                    mySubDir->Nfiles++;
                                }
                                found = 0;
                            }
                        }
                    }
                }
            }
        }
        closedir(passed_dir);
        free(temp);
        //printf("%d READ %s\n",getpid()%10, Dir);
        // parseDir(Universal, Dir);
    }
    free(filename);
    // }
    // free(receivedBuffer);
}

void parse(monitorDataPointer *Universal, char *filename)
{
    int filter, index;
    char *input, **word;
    char *citizenID = NULL, *name = NULL, *surname = NULL, *country = NULL, *age = NULL, *virus = NULL, *vaccinated = NULL, *date = NULL;
    input = (char*) malloc(sizeof(char)*201); 
    word = (char**) malloc(sizeof(char*)*5);

    char **vacced = *(Universal->vaccedPtr); 
    Record **Registry = *(Universal->RegistryPtr);
    int *inputSize = Universal->inputSizePtr;

    FILE *citizenRecordsFile = fopen(filename,"r");
    Record *NodePointer;
    int duplicate = 0; 

    if(citizenRecordsFile==NULL){
        //printf("ERROR: %s did not open\n",filename);
        perror("file");
        exit(1);
    }

    while (fgets(input,200,citizenRecordsFile)!=NULL){
        //input[strcspn(input, "\n")] = 0;  //delete newline character from the string if it exists
        if(input[strlen(input)-1]=='\n') 
            input[strlen(input)-1]='\0';
        citizenID = strtok (input, " \t");
        name = strtok (NULL, " \t");
        surname = strtok (NULL, " \t");
        country = strtok (NULL, " \t");
        age = strtok (NULL, " \t");
        virus = strtok (NULL, " \t");
        vaccinated = strtok (NULL, " \t");
        date = strtok (NULL, " \t");

        if(vaccinated!=NULL)
        {
            if(!strcmp(vaccinated, "YES"))
                word[4]=vacced[1];
            else
                word[4]=vacced[0];
        }
        else
        {
            //printf("ERROR IN RECORD %s %s %s %s %s %s %s %s\n",citizenID, name, surname, country, age, virus, vaccinated, date!=NULL ? date : "");
            continue;
        }

        word[0]=virus;
        word[2]=country;
        if(virus_exists(Universal, &word, &filter, &index)==0)
            create_datatypes(Universal);

        duplicate = existsIn_hashtable(Registry[my_hash(atoi(citizenID))], &NodePointer, citizenID, virus);
        if(!strcmp(word[4], "NO") && date!=NULL){
            // printf("ERROR (date) IN RECORD %s %s %s %s %s %s %s %s\n",citizenID, name, surname, country, age, virus, vaccinated, date);
            continue;
        }
        else if(!duplicate) //no duplicates
        {
            insertAt_hashtable(&Registry[my_hash(atoi(citizenID))], citizenID, name, surname, word[3], age, word[1], word[4], date);
            (*inputSize)++;
        }
        else if(duplicate){
            // printf("ERROR (duplicate) IN RECORD %s %s %s %s %s %s %s %s \n", citizenID, name, surname, country, age, virus, vaccinated, date!=NULL ? date : "");
            continue;
        }
    }
    fclose(citizenRecordsFile);
    free(input);
    free(word);
}

void parseDir(monitorDataPointer* Universal, char *file)
{
    // monitorDir *mySubDir; //= *(Universal->mySubDirPtr); 
    DIR *passed_dir;
    struct dirent *passed_file;
    char *filename = NULL;
    struct stat filestatus; 
    char *dirname = *(Universal->dirnamePtr);    
    char *temp = (char*) malloc(sizeof(char)*(strlen(dirname)+strlen(file)+2));
    // printf("file %s\n",file);
    snprintf(temp, strlen(dirname)+strlen(file)+2, "%s/%s", dirname, file);
    // printf("temp %s\n",temp);
    passed_dir = opendir(temp);
    if (passed_dir == NULL){
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    while((passed_file=readdir(passed_dir)) != NULL)
        if(strcmp(passed_file->d_name,".") && strcmp(passed_file->d_name,"..") && passed_file->d_ino!=0){
            filename = (char*) realloc(filename, strlen(dirname)+2*strlen(file)+10); 
            snprintf(filename, strlen(temp)+strlen(passed_file->d_name)+2 , "%s/%s", temp, passed_file->d_name);
            if(!isFileRead(Universal->mySubDirPtr, passed_file->d_name))
            {
                // printf("%s\n",passed_file->d_name);
                // mySubDir = *(Universal->mySubDirPtr); 
                if (stat(filename, &filestatus) == -1){
                    perror("stat");
                    exit(EXIT_FAILURE);
                }
                else
                    if(filestatus.st_size>1)
                        parse(Universal, filename);
            }
        }
    closedir(passed_dir);
    free(temp);
    free(filename);
}

void storeDataIntoDatatypes(monitorDataPointer* Universal)
{
    Record *NodePointer = NULL;
    Record **Registry = *(Universal->RegistryPtr);
    Sentinel **skiplist = *(Universal->skipListPtr);
    char **bloomfilter = *(Universal->bloomFilterPtr);
    char *citizenID = NULL/*, *name = NULL, *surname = NULL, *country = NULL, *age = NULL, *virus = NULL*/, *vaccinated = NULL;//, *date = NULL;
    char **vacced = *(Universal->vaccedPtr); 
    char **word = (char**) malloc(sizeof(char*)*4);
    char *bloomString = (char*) malloc(sizeof(char));
    int j=0, k=1, filter, index, first=1;
    int inputSize = *(Universal->inputSizePtr);
    // if(inputSize>=2)
    //     k = abs(log2(inputSize));
    k=8; 
    
    // printf("before\n");
    if(Universal->initSL==0)
    {
        for(int i=0; i<*(Universal->NskipListsPtr); i++)
        {            
            skiplist[i]->NSkipLevels = k;
            skiplist[i]->next = (Node**) realloc(skiplist[i]->next, sizeof(Node*)*((long unsigned int) k));
            for(int l=0; l<k; l++)
                skiplist[i]->next[l] = NULL;

        } 
        Universal->initSL=1;
    }  
    // printf("after\n");

    int in=0;
    for(int i=0; i<10; i++)
    {
        // printf("%d of %d\n",i,10);
        k=1;
        j=0;
        while(k)
        {
            k = export2From_hashtable(Registry[i], &NodePointer, &citizenID/*, &name, &surname, &country, &age*/, &word[0], &vaccinated/*, &date*/, j, first);
            if(k==0) break;
            if(first)
                first--;
            //if(NodePointer==NULL)
            //    continue;
            // printf("%d %s\n",in++,citizenID);
            word[2]=NULL;
            virus_exists(Universal, &word, &filter, &index); //is called to get the index of the bloomFilter of the current virus            
            if(vaccinated==vacced[1])
            {
                bloomString = (char*) realloc(bloomString, sizeof(char)*( strlen(citizenID) + strlen(word[1]) + 1 ) );
                snprintf(bloomString, strlen(citizenID)+strlen(word[1]) + 1, "%s%s", citizenID, word[1]);
                insertAt_bloomFilter( bloomfilter,  filter, *(Universal->bloomSizePtr), (unsigned char*) bloomString);
                int bloomCheck = isItemIn_bloomFilter( bloomfilter, filter, *(Universal->bloomSizePtr), (unsigned char*) bloomString);
                // printf("Check for '%s' in filter=%d: %d\n",citizenID,filter,bloomCheck);
                insertAt_skipList(&(skiplist[2*filter]), NodePointer);
                // printll(skiplist[2*filter]->next[0],0);
            }
            else if(vaccinated==vacced[0]){
                insertAt_skipList(&(skiplist[2*filter+1]), NodePointer);
                // printll(skiplist[2*filter+1]->next[0],0);
            }
            j++;
        }
    }
    free(word);
    free(bloomString);
    free(citizenID);
}

void sendBloomFilters(monitorDataPointer *Universal)
{
    int NbloomFilters = *(Universal->NbloomFiltersPtr); 
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    int bloomSize = *(Universal->bloomSizePtr); 
    char **bloomFilter = *(Universal->bloomFilterPtr); 
    char **virusData = *(Universal->virusDataPtr); 
    // int *fifo = *(Universal->fifoPtr); 
    int mySocket = *(Universal->mySocketPtr);
    for(int i=0;i<NbloomFilters;i++)
    {
        // write_hook(fifo[WRITE], virusData[i], makeHeader(2, strlen(virusData[i])), bufferSize, strlen(virusData[i]));
        // write_hook(fifo[WRITE], bloomFilter[i], makeHeader(2, bloomSize), bufferSize, bloomSize);
        write_hook(mySocket, virusData[i], makeHeader(2, strlen(virusData[i])+1), socketBufferSize, strlen(virusData[i])+1);
        write_hook(mySocket, bloomFilter[i], makeHeader(2, bloomSize), socketBufferSize, bloomSize);
        //printf("%d) virus sent = %s with strlen(%d) from %d\n",i, virusData[i], strlen(virusData[i]), getpid());        
        //printf("(%d) virus sent = %s from %d\n\n",i, virusData[i], getpid());
    }
}

void sendReadyMsg(monitorDataPointer *Universal)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    // int *fifo = *(Universal->fifoPtr); 
    char *msg = (char*) malloc_hook(6);
    int mySocket = *(Universal->mySocketPtr);
    strcpy(msg,"READY"); 
    //write_hook(fifo[WRITE], msg, makeHeader(2, 6), bufferSize, 6);
    write_hook(mySocket, msg, makeHeader(2, 6), socketBufferSize, 6);
    free(msg);
    //kill(getppid(),SIGUSR2);
}

int receiveReadyMsg(monitorDataPointer *Universal)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    // int *fifo = *(Universal->fifoPtr); 
    int mySocket = *(Universal->mySocketPtr);
    void *receivedBuffer = malloc_hook(1);
    int success = 0;
    // Monitor* Monitor = *(Universal->monitorPtr);
    //while(1){
    // if(read_hook(fifo[2*monitor+1],&receivedBuffer,bufferSize)!=0)
    if(read_hook(mySocket,&receivedBuffer,socketBufferSize)!=0)
        if(!strcmp(receivedBuffer, "READY")){
            success++;
        }
        // else
        //     printf("%s\n",receivedBuffer);
    free(receivedBuffer);
    return success;
}

void waitForParent(monitorDataPointer *Universal)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    // int *fifo = *(Universal->fifoPtr);
    int mySocket = *(Universal->mySocketPtr);
    void *receivedBuffer = NULL;
    char **argument = malloc_hook(sizeof(char*)*4);
    while(1)
    {
        kill(getpid(), SIGSTOP);
        // printf("SIGCONT\n");
        checkFlag(Universal);
        // if(read_hook(fifo[READ],&receivedBuffer,bufferSize)!=0){
        if(read_hook(mySocket,&receivedBuffer,socketBufferSize)!=0){
            argument[0] = strtok(receivedBuffer,"|");
            argument[1] = strtok(NULL,"|");   //CITIZENID
            argument[2] = strtok(NULL,"|");   //VIRUS
            argument[3] = strtok(NULL,"|");   //DATE
            if(!strcmp(argument[0],"REQUEST"))
                travelRequest(Universal, argument[1], argument[2], argument[3]);
            else if(!strcmp(argument[0],"SEARCH"))
                searchVaccinationStatus(Universal, argument[1]);
            else if(!strcmp(argument[0],"ADDRECORDS"))
                addVaccinationRecords(Universal);
            else if(!strcmp(receivedBuffer, "EXIT")){
                free(receivedBuffer);
                exitNow(&Universal);
                break;
            }
            argument[0] = NULL;   //COMMAND
            argument[1] = NULL;   //CITIZENID
            argument[2] = NULL;   //VIRUS
            argument[3] = NULL;   //DATE
            free_hook(receivedBuffer);
            receivedBuffer = NULL;
        }
    }
    free(argument);
}

void travelRequest(monitorDataPointer *Universal, char *citizenID, char *virusName, char *date)
{
    Sentinel **skiplist = *(Universal->skipListPtr);
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    // int *fifo = *(Universal->fifoPtr);
    int mySocket = *(Universal->mySocketPtr);
    int filter = 0;
    char **word = (char**) malloc(sizeof(char*)*3);
    char *answer = NULL; 
    word[0]=virusName;
    word[2]=NULL;        

    if(virus_exists(Universal, &word, &filter, NULL))
    {
        word[0] = citizenID;
        word[1] = virusName; 
        if(search_skipList(&(skiplist[2*filter]->next[0]), word, (skiplist[2*filter])->NSkipLevels))
        {
            answer = (char*) malloc_hook(sizeof(char)*(strlen(word[2])+6));
            snprintf(answer, strlen(word[2])+6 , "YES|%s|", word[2]); 
            if(recentlyVacced(word[2], date)>0)
                addRequest(Universal, date, "NONE", NULL, 1, 0);
            else
                addRequest(Universal, date, "NONE", NULL, 0, 0);
            // printf("\x1b[32mVACCINATED ON %s\x1b[0m\n", word[2]);
        }
        else
        {
            answer = (char*) malloc_hook(sizeof(char)*(3));
            snprintf(answer, 4 , "NO|");            
            addRequest(Universal, date, "NONE", NULL, 0, 1);  
            // printf("\x1b[31mNOT VACCINATED\x1b[0m\n");         
        }
        // write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
        write_hook(mySocket, answer, makeHeader(2, strlen(answer)+1), socketBufferSize, strlen(answer)+1);
        free(answer);
    }
    else
    {
        printf("Skip List for the virus %s is non-existent.\n",virusName);
    }
    free(word);
}

void searchVaccinationStatus(monitorDataPointer *Universal, char *citizenID)
{
    int socketBufferSize = *(Universal->socketBufferSizePtr); 
    // int *fifo = *(Universal->fifoPtr);
    int mySocket = *(Universal->mySocketPtr);
    int NskipLists = *(Universal->NskipListsPtr);
    char **virusData = *(Universal->virusDataPtr);
    Sentinel **skiplist = *(Universal->skipListPtr);
    Record **Registry = *(Universal->RegistryPtr);
    Record *NodePointer = NULL;
    char **word = (char**) malloc(sizeof(char*)*3);
    // char *answer = NULL;
    char **data = (char**) malloc_hook(sizeof(char*)*((long unsigned int) (NskipLists+2)));
    Header **head = (Header**) malloc_hook( sizeof(Header*)*((long unsigned int)(NskipLists+2)));
    int *size = (int*) malloc_hook(sizeof(int)*((long unsigned int) NskipLists+2));
    for(unsigned int i=0; i<((unsigned int) (NskipLists+2));i++){
        data[i] = NULL;
        head[i] = NULL;
        size[i] = 0;
    }
    if(search_hashtable(Registry[my_hash(atoi(citizenID))],  &NodePointer, citizenID))
    {
        // answer = (char*) malloc_hook(sizeof(char)*(strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2));
        // snprintf(answer, strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2 , "%s|%s|%s|%s|", NodePointer->citizenID, NodePointer->name, NodePointer->surname, NodePointer->country);
        // write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
        // free(answer);
        // answer = (char*) malloc_hook(sizeof(char)*(4+strlen(NodePointer->age)+2));
        // snprintf(answer, 4+strlen(NodePointer->age)+2 , "AGE|%s|", NodePointer->age);
        // write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
        // free(answer);

        // data[0] = (char*) malloc_hook(sizeof(char)*(strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2));
        // snprintf(data[0], strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2 , "%s|%s|%s|%s|", NodePointer->citizenID, NodePointer->name, NodePointer->surname, NodePointer->country);
        data[0] = (char*) malloc_hook(sizeof(char)*(strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2));
        snprintf(data[0], strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2 , "%s %s %s %s ", NodePointer->citizenID, NodePointer->name, NodePointer->surname, NodePointer->country);
        
        head[0] = makeHeader(2, strlen(data[0])+1);
        size[0] = strlen(data[0])+1;

        data[1] = (char*) malloc_hook(sizeof(char)*(4+strlen(NodePointer->age)+2));
        snprintf(data[1], 4+strlen(NodePointer->age)+2 , "AGE %s ", NodePointer->age);
        head[1] = makeHeader(2, strlen(data[1])+1);
        size[1] = strlen(data[1])+1;
        // printf("%s\n%s\n",data[0],data[1]);
        for(int i=0; i<NskipLists; i++)
        {
            word[0] = citizenID;
            if(i%2==0)
            {
                word[1] = virusData[i/2];
                if(search_skipList(&(skiplist[i]->next[0]), word, (skiplist[i])->NSkipLevels))
                {
                    // printf("\x1b[32m%s YES %s\x1b[0m\n", virusData[i/2], word[2]);
                    // answer = (char*) malloc_hook(sizeof(char)*(strlen(virusData[i/2])+strlen(word[2])+7));
                    // snprintf(answer, strlen(virusData[i/2])+strlen(word[2])+7 , "%s|YES|%s|", virusData[i/2], word[2]);
                    // write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
                    // free(answer);
                    data[i+2] = (char*) malloc_hook(sizeof(char)*(strlen(virusData[i/2])+strlen(word[2])+16));
                    snprintf(data[i+2], strlen(virusData[i/2])+strlen(word[2])+16 , "%s VACCINATED ON %s", virusData[i/2], word[2]);
                    head[i+2] = makeHeader(2, strlen(data[i+2])+1);
                    size[i+2] = strlen(data[i+2])+1;
                }
            }
            else if(i%2==1)
            {
                word[1] = virusData[(i-1)/2];
                if(search_skipList(&(skiplist[i]->next[0]), word, (skiplist[i])->NSkipLevels))
                {
                    // printf("\x1b[32m%s no %s\x1b[0m\n", virusData[(i-1)/2]);
                    // answer = (char*) malloc_hook(sizeof(char)*(strlen(virusData[(i-1)/2])+5));
                    // snprintf(answer, strlen(virusData[(i-1)/2])+5 , "%s|NO|", virusData[(i-1)/2]); 
                    // write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
                    // free(answer);
                    data[i+2] = (char*) malloc_hook(sizeof(char)*(strlen(virusData[(i-1)/2])+20));
                    snprintf(data[i+2],  strlen(virusData[(i-1)/2])+20 , "%s NOT YET VACCINATED", virusData[(i-1)/2]); 
                    head[i+2] = makeHeader(2, strlen(data[i+2])+1);
                    size[i+2] = strlen(data[i+2])+1;
                }  
            }
        }
        multiple_write_hook(mySocket, data, head, NskipLists, socketBufferSize, size);
    }
    for(unsigned int i=0; i<((unsigned int) (NskipLists+2));i++){
        free_hook(data[i]);
    }
    free(word);
    free(data);
    free(head);
    free(size);
}

void addVaccinationRecords(monitorDataPointer *Universal)
{
    cyclicBuffer *cyclicbuffer = *(Universal->cyclicbufferPtr);
    receiveSubdirs(Universal,*(Universal->argvPtr));
    monitorDir *mySubDir = *(Universal->mySubDirPtr); 
    int counter = 0;

    while(1)
    {
        // printf("%d\n",counter);
        if(counter>=mySubDir->Nfiles)
            break;
        if(isFileRead(Universal->mySubDirPtr, mySubDir->file[counter])){
            counter++;
            continue;
        }
        // printf("%d/%d: %s\n",counter,mySubDir->Nfiles,mySubDir->file[counter]);
        pthread_mutex_lock(&cyclicMutex);
        if(isWritable_circularBuffer(&cyclicbuffer)){
            // printf("parent wrote %s\n",mySubDir->file[counter]);
            writeAt_circularBuffer(&cyclicbuffer, mySubDir->file[counter]);
            counter++;
        }
        else{  //wait till buffer has space for you to write on
            // printf("par thread got blocked\n");
            pthread_cond_wait(&cyclicWriteCond,&cyclicMutex);
        }
        pthread_mutex_unlock(&cyclicMutex);
        pthread_cond_signal(&cyclicReadCond);
    }
    
    //wait for the parser threads to do their job 
    while(1)    
    {
        pthread_mutex_lock(&cyclicMutex);
        while(isReadable_circularBuffer(&cyclicbuffer)){
            // printf("got blocked\n");
            pthread_cond_wait(&cyclicWriteCond,&cyclicMutex);
        }
        pthread_mutex_unlock(&cyclicMutex);
        break;
    }

    sendBloomFilters(Universal);
    // kill(getpid(),SIGSTOP); 
    sendReadyMsg(Universal);     
    // kill(getpid(),SIGSTOP); 
    return;
}

void exitNow(monitorDataPointer **Universal)
{
    monitorDataPointer *temp = *Universal;
    Sentinel **skiplist = *(temp->skipListPtr);
    Record** Registry = *(temp->RegistryPtr);
    monitorDir *mySubDir = *(temp->mySubDirPtr);
    char **virusData = *(temp->virusDataPtr);
    char **countryData = *(temp->countryDataPtr);
    char **vacced = *(temp->vaccedPtr);
    char **bloomfilter = *(temp->bloomFilterPtr);
    //char *filename = *(temp->dirnamePtr);
    int numThreads = *(temp->numThreadsPtr);
    pthread_t *parser = *(temp->parserPtr);
    int mySocket = *(temp->mySocketPtr);

    mkdir(LOG_DIR, 0777);
    int file;
    int pid = getpid();
    char *filename = malloc(countDigits(pid)+10+strlen(LOG_DIR));
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
    for(int i=0;i<*(temp->NcountriesPtr);i++)
        printf("%s\n",countryData[i]);
    getRequests(temp, NULL, NULL, NULL, NULL, 1);
    dup2(fd, 1);
    close(fd);
    free(filename);

    filename = *(temp->dirnamePtr);
    exitThread = 1;
    pthread_cond_broadcast(&cyclicReadCond);
    for(int i=0;i<numThreads;i++){
        // pthread_kill(parser[i], SIGKILL);
        if(pthread_join(parser[i], NULL)!=0){
            printf("error: pthread_join\n");
            //exit(EXIT_FAILURE);
        }
    }
    free(parser);

    if(shutdown(mySocket, SHUT_RDWR)<0)
        perror("shutdown socket");

    if(close(mySocket)<0)
        perror("close socket"); 
    pthread_mutex_destroy(&cyclicMutex);
    pthread_cond_destroy(&cyclicReadCond);
    pthread_cond_destroy(&cyclicWriteCond);

    int *fifo = *(temp->fifoPtr);
    cyclicBuffer **cyclicbuffer = temp->cyclicbufferPtr;

    reqReg *requestsRegistry = *(temp->requestsRegistryPtr);
    requests *Requests;
    if(requestsRegistry!=NULL){
        for(int i=0;i<requestsRegistry->n;i++){
            Requests = requestsRegistry->array[i];
            free_hook(Requests->countryTo);
            for(int j=0;j<Requests->nt;j++)
                free_hook(Requests->totalTravelRequestsDate[j]);
            free_hook(Requests->totalTravelRequestsDate);
            free_hook(Requests->totalTravelRequestsCtr);
            for(int j=0;j<Requests->na;j++)
                free_hook(Requests->acceptedRequestsDate[j]);
            free_hook(Requests->acceptedRequestsDate);
            free_hook(Requests->acceptedRequestsCtr);
            for(int j=0;j<Requests->nr;j++)
                free_hook(Requests->rejectedRequestsDate[j]);
            free_hook(Requests->rejectedRequestsDate);
            free_hook(Requests->rejectedRequestsCtr);
            free_hook(requestsRegistry->array[i]);
        }
        free_hook(requestsRegistry->array);
        free_hook(requestsRegistry);
    }   

    //free hashTable
    for(int i=0; i<10; i++)
		free_hashtable(Registry[i]);
    free(Registry);
    //free skipList
    for(int i=0; i<*(temp->NskipListsPtr); i++)
        free_skipList(&(skiplist[i]));
    free(skiplist);
    //free bloomFilter
    free_bloomFilter(&bloomfilter, *(temp->NbloomFiltersPtr));
    //free countryData array
    for(int i=0; i<*(temp->NcountriesPtr); i++)
        free(countryData[i]);
    free(countryData);
    //free virusData array
    for(int i=0; i<*(temp->NbloomFiltersPtr); i++)
        free(virusData[i]);
    free(virusData);
    //free vacced array
    for(int i=0; i<2; i++)
        free(vacced[i]);
    free(vacced);
    //
    free(fifo);
    //
    free_circularBuffer(cyclicbuffer);
    //free((cyclicBuffer *) *cyclicbuffer);
    free(*(temp->cyclicbufferPtr));
    //
    for(int i=0;i<mySubDir->Nread_files;i++)
        free(mySubDir->read_files[i]);
    free(mySubDir->read_files);
    for(int i=0;i<mySubDir->Nfiles;i++)
        free(mySubDir->file[i]);
    free(mySubDir->file);
    for(int i=0;i<mySubDir->Nsub_dirs;i++)
        free(mySubDir->sub_dir[i]);
    free(mySubDir->sub_dir);
    free(mySubDir);
    //free filename string
    free(filename);
    //free the Universal Datapointer 
    free(*Universal);
    // printf("\033[1;37mSuccessfuly cleaned monitor %d data.\033[0m\n",getpid()%10);
}
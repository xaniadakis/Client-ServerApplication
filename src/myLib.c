#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sys/epoll.h>
#include <sys/wait.h>


#define FIFO_DIR "../tmp/namedPipes/"
#define LOG_DIR "../log_folder/"

#ifndef DataStructHeader
#define DataStructHeader
#include "../lib/structures/DataStruct.h"
#endif

#ifndef skipStructHeader
#define skipStructHeader
#include "../lib/structures/skipStruct.h"
#endif

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef travelMonitorClientUtilHeader
#define travelMonitorClientUtilHeader
#include "../lib/headers/travelMonitorClientUtil.h"
#endif

extern int errno;

void* malloc_hook(size_t SIZE)
{
    void *ptr;
    if(SIZE<1){
        perror("malloc undefined size");
        return NULL;
    }
    if (!(ptr = malloc(SIZE))){
        perror("malloc");
        exit(1); 
    }
    else return ptr;
}

void* calloc_hook(size_t size, size_t SIZE)
{
    void *ptr;
    if(SIZE<1 || size<1){
        perror("calloc undefined size");
        return NULL;
    }
    if (!(ptr = calloc(size, SIZE))){
        perror("calloc");
        exit(1); 
    }
    else return ptr;
}

void* realloc_hook(void* ptr, size_t SIZE)
{
    void *temp;
    if(SIZE<1){
        perror("realloc undefined size");
        printf("size of realloc = %d\n",(int) SIZE);
        exit(1); 
    }
    if ((temp = realloc(ptr, SIZE))==NULL){
        perror("realloc returned null");
        exit(1); 
    }
    else 
        return temp;
} 

void* allocate(void* ptr, size_t SIZE)
{
    if(ptr==NULL){
        ptr = malloc_hook(SIZE);
    }
    else if(ptr!=NULL){
        ptr = realloc_hook(ptr, SIZE);
    }
    return ptr;
}

void free_hook(void *ptr)
{
    if(ptr!=NULL)
        free(ptr);
}

void usleep_hook(int usec)
{
    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = usec;
    select (0, NULL, NULL, NULL, &timeout);
}

Header* makeHeader(int typeOfData, int sizeOfData)
{
    Header *ptr = (Header*) malloc_hook(sizeof(Header));
    ptr->type = typeOfData;
    ptr->size = sizeOfData;
    return ptr;
}

void write_hook(int pipe, void *data, void *header, int sizeOfBuffer, int sizeOfData)
{
    int err = 0;
    int written = 0;
    int first = 1;
    int nbytes = sizeOfBuffer;
    int headerSize = sizeof(Header);
    char *EndOfFile = NULL;
    void *buffer = malloc_hook(sizeOfBuffer);
    memset(buffer,0,sizeOfBuffer);
    // write Header
    while(written < headerSize)
    {
        if(sizeOfBuffer > headerSize-written)
            nbytes = headerSize-written;
        if(first){
            memcpy(buffer, header, nbytes); 
            first--;
        }else
            memcpy(buffer, header + written, nbytes); 
        written += nbytes;
        if( (err=write(pipe, buffer, nbytes)) != nbytes){
            if((err<0) && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
                continue;
            perror("write"); 
            exit(EXIT_FAILURE);
        }
    }
    free(header);
    nbytes = sizeOfBuffer;
    written = 0;
    first = 1;
    //write message
    while(written < sizeOfData)
    {
        if(sizeOfBuffer > sizeOfData-written)
            nbytes = sizeOfData-written;
        if(first){
            memcpy(buffer, data, nbytes); 
            first--;
        }else
            memcpy(buffer, data + written, nbytes); 
        written += nbytes;
        if( (err=write(pipe, buffer, nbytes)) != nbytes){
            if((err<0) && (errno == EINTR))// || errno == EAGAIN || errno == EWOULDBLOCK))
                continue;
            perror("write"); 
            exit(EXIT_FAILURE);
        }
    }
    //memset(buffer,0,sizeOfBuffer);
    EndOfFile = malloc_hook(sizeof(char)*4);
    strcpy(EndOfFile, "EOF");
    nbytes = sizeOfBuffer;
    written = 0;
    first = 1;
    //send EOF to let the reader now that the current message is over
    while(written < 4)
    {
        if(sizeOfBuffer > 4-written)
            nbytes = 4-written;
        if(first){
            memcpy(buffer, EndOfFile, nbytes); 
            first--;
        }else
            memcpy(buffer, EndOfFile + written, nbytes); 
        written += nbytes;
        if( (err=write(pipe, buffer, nbytes)) != nbytes){
            if((err<0) && (errno == EINTR))// || errno == EAGAIN || errno == EWOULDBLOCK))
                continue;            
            perror("write"); 
            exit(EXIT_FAILURE);
        }
    }
    //printf("wrote %d bytes\n",sizeOfData+4+8);
    free(EndOfFile);
    free(buffer);
}

void multiple_write_hook(int pipe, char** data, Header **header, int n, int sizeOfBuffer, int *sizeOfData)
{
    for(int i=0;i<n;i++)
        if(sizeOfData[i]!=0)
            write_hook(pipe, (void *) data[i], (void *) header[i], sizeOfBuffer, sizeOfData[i]);
}

int read_hook(int pipe, void **receivedBuffer, int sizeOfBuffer)
{
    if(sizeOfBuffer<1)
        sizeOfBuffer=1;
    int n = 0;
    int first = 1;
    //int f = 1;
    int written = 0;
    int nbytes = sizeOfBuffer;
    int sizeOfData = 0;
    int received = 0;
    //int check=0;
    //int *index = malloc_hook(1);
    //fd_set set;
    // struct timeval timeout;
    // timeout.tv_sec = 0;
    // timeout.tv_usec = 100000;
    int fd;
    int retval;
    Header *header = NULL;
    //memset(buffer,0,sizeOfBuffer);
    char *EndOfFile, *temp;
    EndOfFile = malloc_hook(sizeof(char)*4);
    if((*receivedBuffer)!=NULL)
        free((*receivedBuffer));
    (*receivedBuffer) = NULL;
    (*receivedBuffer) = malloc_hook(1);
    void *buffer = malloc_hook(sizeOfBuffer);
    // while(1){
    //     FD_ZERO(&set);             
    //     FD_SET(pipe, &set);     
    //     if (select(pipe+1, &set, NULL, NULL, &timeout) == -1){
    //         perror("select");
    //         exit(EXIT_FAILURE);
    //     }
    //     else if(FD_ISSET(pipe, &set)){
    //         received++;
    //         if(written>=sizeof(Header) && sizeOfData==0)
    //         {
    //             header = (Header*) (*receivedBuffer);
    //             //printf("header sz=%d\n",header->size);
    //             sizeOfData = header->size + sizeof(Header) + 4;
    //         }
    //         if(sizeOfData==0){                                               //read the header
    //             if(sizeOfBuffer>sizeof(Header))                              //if bufferSize is bigger than header read the header
    //                 nbytes=sizeof(Header);
    //             else                                                         //if bufferSize is smaller than header read a chunk of the header
    //                 nbytes=sizeof(Header)-written;
    //         }
    //         else{
    //             if(sizeOfBuffer > sizeOfData-written)                        //if bufferSize is bigger than bytes left to read, only read em
    //                 nbytes = sizeOfData-written;
    //             else if(sizeOfBuffer <= sizeOfData-written)                   //else read bufferSize data
    //                 nbytes = sizeOfBuffer;
    //         }
    //         //printf("times in %d sizeOfData %d written %d and will read %d\n",f++, sizeOfData, written,nbytes);
    //         n = read(pipe, buffer, nbytes);                                  //read n bytes
    //         if(n<0){
    //             perror("read");
    //             exit(EXIT_FAILURE);
    //         }
    //         if(n==0)    break;
    //         written += n;                                                   //count sizeOf received data
    //         (*receivedBuffer) = realloc_hook((*receivedBuffer), written);   //allocate space 
    //         if(first){
    //             memcpy((*receivedBuffer), buffer, n);                       //write received data to a buffer
    //             first--;
    //         }
    //         else
    //             memcpy((*receivedBuffer) + written - n, buffer, n);
    //         if(!sizeOfData)
    //             continue;
            
    //         //check if we 've reached end of file
    //         temp = (char*) (*receivedBuffer);
    //         snprintf(EndOfFile, 4 , "%c%c%c", temp[written-4], temp[written-3], temp[written-2]);
    //         if(!strcmp(EndOfFile,"EOF"))
    //             break;
    //         if(written>=sizeOfData)
    //             break;
            
    //     }
    //     else 
    //         break;
    // }
    struct epoll_event ev, event;
    memset(&ev, 0, sizeof(ev));
    fd = epoll_create1(0);
    if(fd<0) 
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = pipe;
    if(epoll_ctl(fd, EPOLL_CTL_ADD, pipe, &ev)<0) 
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        retval = epoll_pwait(fd, &event, 1, 100, NULL);
        if(retval<0) 
        {
            if(errno!=EINTR)    //ressurect from signal interruption
            {
                perror("epoll_pwait");
                exit(EXIT_FAILURE);
            }
        }
        else if(retval==1)
        {
            if(event.data.fd == pipe)
            {
                received++;
                if(written>=((int) sizeof(Header)) && sizeOfData==0)
                {
                    header = (Header*) (*receivedBuffer);
                    //printf("header sz=%d\n",header->size);
                    sizeOfData = header->size + sizeof(Header) + 4;            
                    // printf("before %d %d %d\n",written, n,sizeOfData);
                    // (*receivedBuffer) = realloc_hook((*receivedBuffer), sizeOfData);
                }
                if(sizeOfData==0){                                               //read the header
                    if(sizeOfBuffer>((int) sizeof(Header)))                              //if bufferSize is bigger than header read the header
                        nbytes=(int) sizeof(Header);
                    else                                                         //if bufferSize is smaller than header read a chunk of the header
                        nbytes=(int) sizeof(Header)-written;
                }
                else{
                    if(sizeOfBuffer > sizeOfData-written)                        //if bufferSize is bigger than bytes left to read, only read em
                        nbytes = sizeOfData-written;
                    else if(sizeOfBuffer <= sizeOfData-written)                   //else read bufferSize data
                        nbytes = sizeOfBuffer;
                }
                if(nbytes>sizeOfBuffer)
                    nbytes=sizeOfBuffer;
                //printf("times in %d sizeOfData %d written %d and will read %d\n",f++, sizeOfData, written,nbytes);
                n = read(pipe, buffer, nbytes);                                  //read n bytes
                if(n<0){
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                if(n==0)    break;
                written += n;                                                   //count sizeOf received data
                // printf("before %d %d %d\n",written, n,sizeOfData);
                if(*receivedBuffer==NULL){
                    perror("memory error");
                    exit(EXIT_FAILURE);
                }
                (*receivedBuffer) = realloc_hook((*receivedBuffer), written);   //allocate space
                // printf("after\n"); 
                if(first){
                    memcpy((*receivedBuffer), buffer, n);                       //write received data to a buffer
                    first--;
                }
                else
                    memcpy((*receivedBuffer) + written - n, buffer, n);
                if(!sizeOfData)
                    continue;
                
                //check if we 've reached end of file
                temp = (char*) (*receivedBuffer);
                snprintf(EndOfFile, 4 , "%c%c%c", temp[written-4], temp[written-3], temp[written-2]);
                if(!strcmp(EndOfFile,"EOF"))
                    break;
                if(written>=sizeOfData)
                    break;
            }
        }
        else if(retval==0)
            break;
    }

    free(EndOfFile);
    free(buffer);
    if(received && written>0){
        void *tempBuffer = malloc_hook(sizeOfData - sizeof(Header) - 4);
        memmove(tempBuffer, (*receivedBuffer) + sizeof(Header), sizeOfData - sizeof(Header) - 4);
        free(*receivedBuffer);
        (*receivedBuffer) = malloc_hook(sizeOfData - sizeof(Header) - 4);
        memmove((*receivedBuffer), tempBuffer, sizeOfData - sizeof(Header) - 4);
        free(tempBuffer);
        return written - sizeof(Header) - 4;
    }
    else
        return 0;
}

//simple coinFlip function
int coinFlip()
{
    return (rand()%INT_MAX)%2;
}

//this function checks if current virus is stored in the array virusData and if current country is stored in the array countryData
//if they do not exist it creates them and informs the caller about their position in the arrays with the filter and index variables
//returns 1 if virus already exists and 0 if virus didn't exist in the virusData array so that new skipList and bloomFilter can be allocated for the current virus
//receives virus in word[0] and country in word[2] and returns pointers to the virusData array in word[1] and to the countryData array in word[3]
int virus_exists(monitorDataPointer *Universal, char ***word, int *filter, int *index)
{
    //printf("virus=%s and country=%s\n", (*word)[0], (*word)[2]);         
    char ***virusData = Universal->virusDataPtr;
    char ***countryData = Universal->countryDataPtr;
    int Nviruses = *(Universal->NbloomFiltersPtr);
    int *Ncountries = Universal->NcountriesPtr;
    int n = Nviruses;
    int nc = *Ncountries;
    int countryFound = 0;
    int virusFound = 0;
    //allocate the char array that will store the names of the viruses
    if((*virusData)==NULL && Nviruses==0){
        (*virusData) = (char**) malloc(sizeof(char*)*1);
        (*virusData)[0] = (char*) malloc(sizeof(char)*(strlen((*word)[0])+1));
        strcpy((*virusData)[0], (*word)[0]);
        (*filter) = 0;
        (*word)[1] = (*virusData)[0];
        //printf("virus =%s and virusData =%s\n", (*word)[1], (*virusData)[0]);         
        //return 0;
    }                
    //allocate the char array that will store the names of the countries
    if((*word)[2]!=NULL)
    {
        if(countryData!=NULL && nc==0){
            (*countryData) = (char**) malloc(sizeof(char*)*1);    
            (*countryData)[0] = (char*) malloc(sizeof(char)*(strlen((*word)[2])+1));    
            strcpy((*countryData)[0], (*word)[2]);    
            (*index)=0;    
            (*word)[3] = (*countryData)[0];    
            //printf("country =%s and countryData =%s\n", (*word)[3], (*countryData)[0]); 
            (*Ncountries)++;        
        }
    }                
    //search if country exists in the array
    if((*word)[2]!=NULL)
    {
        for(int i=0; i<nc; i++)
        {
            if(!strcmp((*word)[2], (*countryData)[i])){
                (*index)=i;
                countryFound = 1;
                (*word)[3] = (*countryData)[i];
            }
        }
    }         
    //search if virus exists in the array
    for(int i=0; i<n; i++)
    {
        if(!strcmp((*word)[0], (*virusData)[i]))
        {
            (*filter)=i;
            virusFound = 1;
            (*word)[1] = (*virusData)[i];
            //return 1;
        }    
    }              
    if(n>0)
    {
        //if virus dont exist in the array put it
        if(!virusFound)
        {
            (*virusData) = (char**) realloc((*virusData), sizeof(char*)*(n+1));
            (*virusData)[n] = (char*) malloc(sizeof(char)*(strlen((*word)[0])+1));
            strcpy((*virusData)[n], *word[0]);
            (*filter)=n;
            (*word)[1] = (*virusData)[n];
            //printf("virus =%s and virusData =%s\n", (*word)[1], (*virusData)[n]);         

        } 
        //if country dont exist in the array put it
        if(!countryFound && (*word)[2]!=NULL)
        {
            (*countryData) = (char**) realloc((*countryData), sizeof(char*)*(nc+1));
            (*countryData)[nc] = (char*) malloc(sizeof(char)*(strlen((*word)[2])+1));
            strcpy((*countryData)[nc], (*word)[2]);
            (*index)=nc;
            (*word)[3] = (*countryData)[nc];
            //printf("country =%s and countryData =%s\n", (*word)[3], (*countryData)[nc]); 
            (*Ncountries)++;
        } 
    } 
    //printf("virusout =%s and countryout=%s\n", (*word)[1], (*word)[3]);         
    if(virusFound)
        return 1;
    return 0;
}

//creates new bloomFilter and skipLists
void create_datatypes(monitorDataPointer *Universal)
{
    char ***bloomFilter = Universal->bloomFilterPtr;
    Sentinel ***skipList = Universal->skipListPtr;
    int *Nbloom = Universal->NbloomFiltersPtr;
    int *Nskip = Universal->NskipListsPtr;
    int bloomSize = *(Universal->bloomSizePtr);
    int inputSize = *(Universal->inputSizePtr);
    int nb = *Nbloom;
    //i initialize skipLists with 1 level and I then alter it according to the inputSize
    int skipLevels=1;
    if(inputSize>=2)
        skipLevels = abs(log2(inputSize));

    (*Nbloom)++;
    int ns = *Nskip;
    (*Nskip) = (*Nskip)+2;
    if(nb==0 && ns==0)
    {   
        (*bloomFilter) = (char**) malloc(sizeof(char*)*1);    
        (*skipList) = (Sentinel**) malloc(sizeof(Sentinel*)*2);
    }
    else
    {
        (*bloomFilter) = (char**) realloc((*bloomFilter), sizeof(char*)*(nb+1));
        (*skipList) = (Sentinel**) realloc((*skipList), sizeof(Sentinel*)*(ns+2));
    }
    (*bloomFilter)[nb] = (char*) calloc(1,sizeof(char)*bloomSize);
    (*skipList)[ns] = (Sentinel*) malloc(sizeof(Sentinel));
    (*skipList)[ns]->next = (Node**) malloc(sizeof(Node*)*skipLevels);
    (*skipList)[ns]->data = NULL;
    (*skipList)[ns]->NSkipLevels = skipLevels;
    (*skipList)[ns+1] = (Sentinel*) malloc(sizeof(Sentinel));
    (*skipList)[ns+1]->next = (Node**) malloc(sizeof(Node*)*skipLevels);
    (*skipList)[ns+1]->data = NULL;
    (*skipList)[ns+1]->NSkipLevels = skipLevels;
    for(int i=0; i<skipLevels; i++){
        (*skipList)[ns]->next[i] = NULL;
        (*skipList)[ns+1]->next[i] = NULL;
    }
    //printf("Just created %d bloomfilter and %d,%d linkedlists\n",nb,ns,ns+1);
}

//compares two dates 
//returns 1 if date1>date2
//returns -1 if date1<date2
//returns 0 if date1==date2
int compare_dates(int d1, int m1, int y1, int d2, int m2, int y2)
{
    if(y1>y2)
        return 1;
    else if(y1<y2)
        return -1;
    else if(y1==y2)
    {
        if(m1>m2)
            return 1;
        else if(m1<m2)
            return -1;
        else if(m1==m2)
        {
            if(d1>d2)
                return 1;
            else if(d1<d2)
                return -1;
            else if(d1==d2)
                return 0;
        }
    }
    return -2;
}

//returns 1 if date1 <= date <= date2
int in_dateSpan(int d1, int m1, int y1, char *date, int d2, int m2, int y2)
{
    int d, m ,y;
    if(date==NULL) 
        return -1;
    d = atoi(strtok (date,"-/"));
    m = atoi(strtok (NULL, "-/"));
    y = atoi(strtok (NULL, "-/"));
    //printf("'%d' '%d' '%d' \n",d1,m1,y1);
    //printf("'%d' '%d' '%d' \n",d,m,y);
    //printf("'%d' '%d' '%d' \n",d2,m2,y2);
    if(compare_dates(d,m,y,d1,m1,y1)>=0 && compare_dates(d2,m2,y2,d,m,y)>=0)
        return 1;
    else 
        return -1;
}

//returns 1 if date1 <= date <= date2
int in_dateSpan2(char *date1, char *date, char *date2)
{
    int d1, m1, y1, d, m, y, d2, m2, y2;
    char* temp;
    if(date1==NULL || date==NULL || date2==NULL) 
        return -1;
    // if(date1==NULL&&date2==NULL)
    //     return -1;
    temp = strtok (date,"-/");
    if(temp!=NULL) d = atoi(temp);
    else return -1;
    temp = strtok (NULL,"-/");
    if(temp!=NULL) m = atoi(temp);
    else return -1;
    temp = strtok (NULL,"-/");
    if(temp!=NULL) y = atoi(temp);
    else return -1;

    temp = strtok (date1,"-/");
    if(temp!=NULL) d1 = atoi(temp);
    else return -1;
    temp = strtok (NULL,"-/");
    if(temp!=NULL) m1 = atoi(temp);
    else return -1;
    temp = strtok (NULL,"-/");
    if(temp!=NULL) y1 = atoi(temp);
    else return -1;

    temp = strtok (date2,"-/");
    if(temp!=NULL) d2 = atoi(temp);
    else return -1;
    temp = strtok (NULL,"-/");
    if(temp!=NULL) m2 = atoi(temp);
    else return -1;
    temp = strtok (NULL,"-/");
    if(temp!=NULL) y2 = atoi(temp);
    else return -1;

    //printf("'%d' '%d' '%d' \n",d1,m1,y1);
    //printf("'%d' '%d' '%d' \n",d,m,y);
    //printf("'%d' '%d' '%d' \n",d2,m2,y2);
    if(compare_dates(d,m,y,d1,m1,y1)>=0 && compare_dates(d2,m2,y2,d,m,y)>=0)
        return 1;
    else 
        return -1;
}

int recentlyVacced(char *dateOfVaccination, char *travelDate)
{
    int dV, mV ,yV, dT, mT, yT;
    if(dateOfVaccination==NULL || travelDate==NULL) 
        return -1;
    char* str = malloc_hook(strlen(dateOfVaccination)+1);
    strcpy(str,dateOfVaccination);
    dV = atoi(strtok (str,"-/"));
    mV = atoi(strtok (NULL, "-/"));
    yV = atoi(strtok (NULL, "-/"));
    free_hook(str);
    dT = atoi(strtok (travelDate,"-/"));
    mT = atoi(strtok (NULL, "-/"));
    yT = atoi(strtok (NULL, "-/"));
    if(dT<dV)    
        mT=mT-1;
    if (mT<mV){
        mT=mT+12;
        yT=yT-1;
    }
    if(yT-yV<0)
        return 0;
    else if(yT-yV==0)
    {
        if(mT-mV>=6)
            return 0;
        else
            return 1;
    }
    else if(yT-yV>0)
        return 0;
    return 0;
}

//if number is NaN return 0.0
float value(float number)
{
    if(number!=number)
        return 0.0;
    else
        return number;
}

int countDigits(int number)
{
    int i = 0;
    while(number>9)  
    {      
        number=number/10;  
        i++;  
    } 
    i++;
    return i; 
}

int isFileRead(monitorDir **mySubDir, char *file)
{
    if((*mySubDir)->Nread_files)
    {
        for(int i=0;i<(*mySubDir)->Nread_files;i++)
            if(!strcmp((*mySubDir)->read_files[i],file))
                return 1;
    }
    if((*mySubDir)->Nread_files==0)
        (*mySubDir)->read_files = (char**) malloc_hook(sizeof(char*)*1);
    else
        (*mySubDir)->read_files = (char**) realloc_hook((*mySubDir)->read_files, sizeof(char*)*((*mySubDir)->Nread_files+1));
    (*mySubDir)->read_files[(*mySubDir)->Nread_files] = (char*) malloc_hook(sizeof(char)*(strlen(file)+1));
    strcpy((*mySubDir)->read_files[(*mySubDir)->Nread_files] , file);                
    // printf("%d last %s\n",getpid(), (*mySubDir)->read_files[(*mySubDir)->Nread_files]);
    (*mySubDir)->Nread_files++;  
    return 0; 
}

void deconstructMsg(char *message, char ***buffer, int *n, int SIZE)
{
    // int i = 0;
    // // char *ptr;
    char *msg = (char*) malloc_hook(strlen(message)+1);
    strcpy(msg,message);
    // (*buffer) = malloc_hook(5);
    // while(1)
    // {
    //     if(i==0){        
    //         // (*buffer) = malloc_hook(1);
    //         (*buffer)[i] = strtok(msg,"|");
    //     }
    //     else{        
    //         // (*buffer) = realloc_hook((*buffer), i+1);            
    //         (*buffer)[i] = strtok(NULL,"|");
    //     }
    //     if((*buffer)[i]==NULL)
    //     {
    //         if(i==0){        
    //             free((*buffer));
    //             (*n) = 0;
    //             return;
    //         }
    //         else{                  
    //             // (*buffer) = realloc_hook((*buffer), i); 
    //             (*n) = i;
    //             return;
    //         }
    //     }
    //     i++;
    // }
    (*buffer) = NULL;
    int idx=0;
    int counter=0;
    for(int i=0;i<SIZE;i++)
    {
        if(msg[i]=='|')
        {
            if((*buffer)==NULL)
                (*buffer) = malloc_hook(1);
            else
                (*buffer) = realloc_hook((*buffer), idx+1);
            (*buffer)[idx] = malloc_hook(i-counter+1);
            memcpy((*buffer)[idx], message+counter, i);
            (*buffer)[idx][i-counter] = '\0';
            printf("%s\n",(*buffer)[idx]);
            counter = i+1;
            (*n)++;
            idx++;
        }
        printf("%d of %d\n",i,SIZE);
    }
    printf("out\n");
    return;
}

char* findBloomFilter(parentDataPointer *Universal, char *countryFrom, char *virusName, int *retval)
{
    Monitor **monitor = *(Universal->monitorPtr);
    int numMonitors = *(Universal->numMonitorsPtr);
    int manager = -1;
    //find which monitor handles countryFrom
    if(countryFrom!=NULL) 
        for(int i=0; i<numMonitors; i++)
        {
            for(int j=0; j<monitor[i]->Nsub_dirs; j++)
            {
                if(!strcmp(monitor[i]->sub_dir[j], countryFrom)){            
                    // printf("%s\n",(monitor[i]).sub_dir[j]);
                    manager = i;
                    (*retval) = i;
                    break;
                }
            }
            if(manager>-1)
                break;
        }
    if(manager==-1)
        return NULL;
    //find which bloomFilter handles virusName 
    if(virusName!=NULL) 
        for(int i=0; i<monitor[manager]->NbloomFilters; i++)
        {            
            //printf("%s\n",((monitor[manager]).bloomFilter[i])->virus);
            if(!strcmp(monitor[manager]->bloomFilter[i]->virus, virusName))
                return monitor[manager]->bloomFilter[i]->bloomFilter;
        }
    return NULL;
}

void addRequest(void *Universal, char *date, char *countryTo, char *virusName, int action, int function)
{
    int found = -1;
    reqReg **requestsRegistry;
    if(function==0)
        requestsRegistry = ((parentDataPointer*) Universal)->requestsRegistryPtr;
    else
        requestsRegistry = ((monitorDataPointer*) Universal)->requestsRegistryPtr;

    requests **Requests;
    if(requestsRegistry!=NULL){
        for(int i=0;i<(*requestsRegistry)->n;i++){
            if(!strcmp((*requestsRegistry)->array[i]->countryTo, countryTo)){
                found = i;
                Requests = &((*requestsRegistry)->array[i]);
            }
        }
    }

    if(found==-1){
        if((*requestsRegistry)->array==NULL)
            (*requestsRegistry)->array = (requests**) malloc_hook(sizeof(requests*));
        else
            (*requestsRegistry)->array = (requests**) realloc_hook((*requestsRegistry)->array, ((*requestsRegistry)->n+1)*sizeof(requests*));
        (*requestsRegistry)->array[(*requestsRegistry)->n] = (requests*) malloc_hook(sizeof(requests));
        Requests = &((*requestsRegistry)->array[(*requestsRegistry)->n]);
        (*Requests)->countryTo = malloc_hook(strlen(countryTo)+1);
        strcpy((*Requests)->countryTo, countryTo);
        (*Requests)->nt = 0;
        (*Requests)->na = 0;
        (*Requests)->nr = 0;
        (*Requests)->totalTravelRequestsDate = NULL;
        (*Requests)->totalTravelRequestsCtr = NULL;
        (*Requests)->acceptedRequestsDate = NULL;
        (*Requests)->acceptedRequestsCtr = NULL;
        (*Requests)->rejectedRequestsDate = NULL;
        (*Requests)->rejectedRequestsCtr = NULL;
        (*requestsRegistry)->n += 1;
    }

    found = -1;
    for(int i=0; i<(*Requests)->nt;i++)
    {
        if(!strcmp((*Requests)->totalTravelRequestsDate[i], date))
            found = i;
    }    

    if(found==-1){           
        (*Requests)->totalTravelRequestsDate = (char**) allocate((*Requests)->totalTravelRequestsDate, ((*Requests)->nt+1)*sizeof(char*));
        (*Requests)->totalTravelRequestsCtr = (int*) allocate((*Requests)->totalTravelRequestsCtr, ((*Requests)->nt+1)*sizeof(int));
        (*Requests)->totalTravelRequestsDate[(*Requests)->nt] = malloc_hook(strlen(date)+1);
        strcpy((*Requests)->totalTravelRequestsDate[(*Requests)->nt], date);
        (*Requests)->totalTravelRequestsCtr[(*Requests)->nt] = 1;
        (*Requests)->nt++;
    }
    else{
        (*Requests)->totalTravelRequestsCtr[found] += 1;
    }

    found = -1;
    if(action==0){
        for(int i=0; i<(*Requests)->nr;i++)
        {
            if(!strcmp((*Requests)->rejectedRequestsDate[i], date))
                found = i;
        }
        if(found==-1){ 
            (*Requests)->rejectedRequestsDate = (char**) allocate((*Requests)->rejectedRequestsDate, ((*Requests)->nr+1)*sizeof(char*));
            (*Requests)->rejectedRequestsCtr = (int*) allocate((*Requests)->rejectedRequestsCtr, ((*Requests)->nr+1)*sizeof(int));
            (*Requests)->rejectedRequestsDate[(*Requests)->nr] = malloc_hook(strlen(date)+1);
            strcpy((*Requests)->rejectedRequestsDate[(*Requests)->nr], date);
            (*Requests)->rejectedRequestsCtr[(*Requests)->nr] = 1;
            (*Requests)->nr++;
        }
        else
            (*Requests)->rejectedRequestsCtr[found] += 1;
    }
    else{
        for(int i=0; i<(*Requests)->na;i++)
        {
            if(!strcmp((*Requests)->acceptedRequestsDate[i], date))
                found = i;
        }
        if(found==-1){ 
            (*Requests)->acceptedRequestsDate = (char**) allocate((*Requests)->acceptedRequestsDate, ((*Requests)->na+1)*sizeof(char*));
            (*Requests)->acceptedRequestsCtr = (int*) allocate((*Requests)->acceptedRequestsCtr, ((*Requests)->na+1)*sizeof(int));
            (*Requests)->acceptedRequestsDate[(*Requests)->na] = malloc_hook(strlen(date)+1);
            strcpy((*Requests)->acceptedRequestsDate[(*Requests)->na], date);
            (*Requests)->acceptedRequestsCtr[(*Requests)->na] = 1;
            (*Requests)->na++;
        }
        else
            (*Requests)->acceptedRequestsCtr[found] += 1;
    }
}

void getRequests(void *Universal, char *date1, char *date2, char *countryTo, char *virusName, int function)
{
    int found = -1;
    int counter = 0;

    reqReg **requestsRegistry = NULL;
    if(function==0)
        requestsRegistry = ((parentDataPointer*) Universal)->requestsRegistryPtr;
    else
        requestsRegistry = ((monitorDataPointer*) Universal)->requestsRegistryPtr;

    requests *Requests;
    if(countryTo!=NULL){
        if(requestsRegistry!=NULL && (*requestsRegistry)!=NULL){
            for(int i=0;i<(*requestsRegistry)->n;i++){
                Requests = (*requestsRegistry)->array[i];
                if(Requests!=NULL&&Requests->countryTo!=NULL&&!strcmp(Requests->countryTo, countryTo)){
                    found = i;
                    break;
                }
            }
        }
    }
    else
    {
        counter = 0;
        if(requestsRegistry!=NULL && (*requestsRegistry)!=NULL)
            for(int k=0;k<(*requestsRegistry)->n;k++){
                Requests = (*requestsRegistry)->array[k];
                for(int i=0; i<Requests->nt;i++){
                    if(date1==NULL||date2==NULL)
                        counter += Requests->totalTravelRequestsCtr[i];
                    else if(in_dateSpan2(date1, Requests->totalTravelRequestsDate[i], date2))
                        counter += Requests->totalTravelRequestsCtr[i];
                }
            }
        printf("TOTAL REQUESTS %d\n",counter);

        counter = 0;
        if(requestsRegistry!=NULL && (*requestsRegistry)!=NULL)
            for(int k=0;k<(*requestsRegistry)->n;k++){
                Requests = (*requestsRegistry)->array[k];
                for(int i=0; i<Requests->na;i++){
                    if(date1==NULL||date2==NULL)
                        counter += Requests->totalTravelRequestsCtr[i];
                    else if(in_dateSpan2(date1, Requests->acceptedRequestsDate[i], date2))
                        counter += Requests->acceptedRequestsCtr[i];
                }
            }
        printf("ACCEPTED %d\n",counter);

        counter = 0;
        if(requestsRegistry!=NULL && (*requestsRegistry)!=NULL)
            for(int k=0;k<(*requestsRegistry)->n;k++){
                Requests = (*requestsRegistry)->array[k];
                for(int i=0; i<Requests->nr;i++){
                    if(date1==NULL||date2==NULL)
                        counter += Requests->totalTravelRequestsCtr[i];
                    else if(in_dateSpan2(date1, Requests->rejectedRequestsDate[i], date2))
                        counter += Requests->rejectedRequestsCtr[i];
                }
            }
        printf("REJECTED %d\n",counter);
    }
    if(countryTo!=NULL){
        if(found==-1){
            printf("No destination country named %s in my records\n", countryTo);
            return;
        }
        else{

            counter = 0;
            for(int i=0; i<Requests->nt;i++){
                if(date1==NULL||date2==NULL)
                    counter += Requests->totalTravelRequestsCtr[i];
                else if(in_dateSpan2(date1, Requests->totalTravelRequestsDate[i], date2))
                    counter += Requests->totalTravelRequestsCtr[i];
            }
            printf("TOTAL REQUESTS %d\n",counter);

            counter = 0;
            for(int i=0; i<Requests->na;i++){
                if(date1==NULL||date2==NULL)
                    counter += Requests->totalTravelRequestsCtr[i];
                else if(in_dateSpan2(date1, Requests->acceptedRequestsDate[i], date2))
                    counter += Requests->acceptedRequestsCtr[i];
            }
            printf("ACCEPTED %d\n",counter);

            counter = 0;
            for(int i=0; i<Requests->nr;i++){
                if(date1==NULL||date2==NULL)
                    counter += Requests->totalTravelRequestsCtr[i];
                else if(in_dateSpan2(date1, Requests->rejectedRequestsDate[i], date2))
                    counter += Requests->rejectedRequestsCtr[i];
            }
            printf("REJECTED %d\n",counter);
        }
    }
}
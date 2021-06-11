#include <stdio.h>

#ifndef DataStructHeader
#define DataStructHeader
#include "../structures/DataStruct.h"
#endif

#ifndef skipStructHeader
#define skipStructHeader
#include "../structures/skipStruct.h"
#endif

void *Main(void *);
void *threadParse(void *);
void uploadMonitorData(monitorDataPointer **, Record ***, Sentinel ***, char ***, char ***,
                       char ***, char ***, char **, int *, int *, int *, int *, 
                       int *, int *, int *, monitorDir **,
                       int **, int *, int *, int *, reqReg **, char ***, cyclicBuffer**, pthread_t**);
void parametersCheck(monitorDataPointer* , int, char **);
void receiveInfo(monitorDataPointer*);
void receiveSubdirs(monitorDataPointer*, char**);                         
void parse(monitorDataPointer*, char*);
void parseDir(monitorDataPointer*, char*);
void storeDataIntoDatatypes(monitorDataPointer*);
void sendBloomFilters(monitorDataPointer*);
void sendReadyMsg(monitorDataPointer*);
int receiveReadyMsg(monitorDataPointer*);
void waitForParent(monitorDataPointer*);
void travelRequest(monitorDataPointer*, char*, char*, char*);
void searchVaccinationStatus(monitorDataPointer*, char*);
void addVaccinationRecords(monitorDataPointer*);
void exitNow(monitorDataPointer**);

#include <stdio.h>
#include <math.h>

#ifndef DataStructHeader
#define DataStructHeader
#include "../structures/DataStruct.h"
#endif

#define K 16

void uploadParentData(parentDataPointer **, int *, int *, char **, int *, int *, 
                      int *, Monitor ***, struct sigaction *, int **, int **, char ***, 
                      reqReg **, char ***);
void parametersCheck(parentDataPointer*, int, char**);
void initMonitorDetails(parentDataPointer*);
void deleteOldLogFiles();
void createSockets(parentDataPointer*);
void fork_monitor(parentDataPointer*, int);
void passInfo(parentDataPointer*, int);
void distributeSubdirs(parentDataPointer*, int);
void receiveBloomFilter(parentDataPointer*, int);
void updateBloomFilter(parentDataPointer*, int);
void sendReadyMsg(parentDataPointer*, int);
int receiveReadyMsg(parentDataPointer*, int); 
void readInputFromKeyboard(parentDataPointer*);
void travelRequest(parentDataPointer*, char*, char*, char*, char*, char*);
void travelStats(parentDataPointer*, char*, char*, char*, char*);
void addVaccinationRecords(parentDataPointer*, char*);
void searchVaccinationStatus(parentDataPointer*, char*);
void kill_hook(parentDataPointer*, char*, char*);
void exitNow(parentDataPointer**,int);

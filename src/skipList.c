#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef skipStructHeader
#define skipStructHeader
#include "../lib/structures/skipStruct.h"
#endif

#ifndef hashTableStructHeader
#define hashTableStructHeader
#include "../lib/structures/hashTableStruct.h"
#endif

void printll(Node* temp, int pp)
{
    if (temp==NULL){ 
        //printf("EMPTY LIST\n");
        return;
    }
    while (temp != NULL)
    {
        if(temp->data!=NULL)
            // printf("%s %s %s %s %s\n", temp->data->citizenID, temp->data->name, 
            // temp->data->surname, temp->data->country, temp->data->age);
            printf("%s %s %s %s %s %s %s\n", temp->data->citizenID, temp->data->name, 
            temp->data->surname, temp->data->country, temp->data->age, temp->data->virus, temp->data->vaccinated);
        temp = temp->next[pp];
    }
}

void insertAt_skipList(Sentinel** skipList, Record* NodePointer)
{
    int skipLevels = (*skipList)->NSkipLevels;
    Node **headLevel, *prev, *temp;
    if(NodePointer==NULL)
        return;
    Node *node = (Node*) malloc(sizeof(Node));
    node->next = (Node**) malloc(sizeof(Node*)*skipLevels);
    node->data = NodePointer;
    for(int i=0; i<skipLevels; i++)
        node->next[i] = NULL;
    int randSkipLvL=1;

    //determine the level at which the newNode will be inserted
    for(int i=1; i<skipLevels; i++)
    {
        if(coinFlip())
            randSkipLvL++;
        else
            break; 
    }
    //insert newNode in the skipList
    for(int i=skipLevels-1; i>=0; i--)
    {
        /*if(i>0){
            if(!coinFlip()){
                Universal->zeros++;
                break;
            }
            else
                Universal->ones++;
        }*/
        /*if(i>0){
            if(!coinFlip())
            {
                Universal->zeros++;
                continue;
            }
            else
                Universal->ones++;
        }*/
        if(i>=randSkipLvL)
            continue;
        //printf("insert %s in level %d where rskl=%d\n",NodePointer->citizenID,i,randSkipLvL);
        //pointers at head of level i of skipList
        headLevel = &(*skipList)->next[i];
        prev = temp = (*skipList)->next[i];

        //if linkedList at level i is empty insert newNode at the beggining
        if((*headLevel)==NULL) 
            (*headLevel) = node;
        //if current citizenID(key) is bigger than the newNode's citizenID(key), insert newNode before it 
        else if(atoi((*headLevel)->data->citizenID)>=atoi(NodePointer->citizenID))
        {
            prev = (*headLevel);
            (*headLevel) = node;
            (*headLevel)->next[i] = prev;
        }
        else
        {
            //while current citizenID(key) is smaller than the newNode's citizenID(key), keep traversing the linkedList
            while((temp->next[i]!=NULL)&&(atoi(temp->data->citizenID)<atoi(NodePointer->citizenID)))
            {
                prev = temp;
                temp = temp->next[i]; 
            }
            //if current citizenID(key) is bigger than the newNode's citizenID(key), go one step back
            if(atoi(temp->data->citizenID)>atoi(NodePointer->citizenID))
                temp = prev;
            //connect the tail of the linkedList to the newNode
            node->next[i] = temp->next[i];
            //insert the newNode at the given position 
            temp->next[i] = node;  
        }
    }
}

void deleteFrom_skipList(Sentinel** skipList, Record* delNode)
{
    Node **headLevel, *temp, *previous;
    Node *toDelete = NULL;
    for(int i=(*skipList)->NSkipLevels-1; i>=0; i--)
    {
        headLevel = &(*skipList)->next[i];
        previous = temp = (*skipList)->next[i];
        //traverse the linkedList of level i to remove the delNode from it
        //if delNode is at the head, delete
        if((*headLevel)!=NULL && atoi(delNode->citizenID)==atoi((*headLevel)->data->citizenID)) 
        {
            //printf("deleted %s, %s looks at %s\n",(*headLevel)->data->citizenID, (*headLevel),temp->next[i] );
            (*headLevel) = temp->next[i];
            toDelete = temp;
        }
        else
            while(1)
            {        
                //if i have reached the end of the linkedList of current level go to the next level
                if(temp==NULL){
                    break;
                }
                //if insertion (citizenID)key is bigger than current node's key, then move forward
                else if(atoi(delNode->citizenID)>atoi(temp->data->citizenID))
                {
                    previous = temp;
                    temp = temp->next[i];
                }
                //found, delete
                else if(atoi(delNode->citizenID)==atoi(temp->data->citizenID))
                {
                    //printf("previous %s looks at %s at level %d\n",(previous!=NULL) ? previous->data->citizenID : previous, (previous->next[i]!=NULL) ? previous->next[i]->data->citizenID : NULL, i);
                    previous->next[i] = temp->next[i];
                    toDelete = temp;
                    //printf("previous %s looks at %s at level %d\n",(previous!=NULL) ? previous->data->citizenID : previous, (temp->next[i]!=NULL) ? temp->next[i]->data->citizenID : temp->next[i], i);
                    break;
                }
                //if insertion (citizenID)key is smaller than current node's key, then stop searching, it dont exist in the current linkedList 
                else if(atoi(delNode->citizenID)<atoi(temp->data->citizenID))
                {
                    //printf("%s bigger than %s\n",delNode->citizenID,temp->data->citizenID);
                    break;
                }
            }
    }
    if(toDelete!=NULL)
    {
        free(toDelete->next);
        free(toDelete);
    }
}

int search_skipList(Node** linkedList, char **word, int N)
{
    Node *temp = *linkedList;
    int skipLevel = N-1;
    if(skipLevel<0)
        skipLevel++;
    //printf("Skiplevels is %d\n",N);
    // int found = 0;

    if (temp==NULL)
        return 0;
    else if(strcmp(temp->data->citizenID, word[0])==0 && strcmp(temp->data->virus, word[1])==0)
    {           
 
        // found=1;
        //word[2]=(char*) malloc(sizeof(char)*(strlen(temp->data->date)+1)); 
        //strcpy(word[2], temp->data->date);
        word[2]=temp->data->date;
        return 1;
    }
    while(1)
    {
        if(temp->next[skipLevel]==NULL)
        {        
            //printf("1: skiplevel %d",skipLevel);
            if(skipLevel>0){
                skipLevel--;
                //printf("down to %d\n",skipLevel);
                continue;
            }
            else{
                //printf("\n");
                break;
            }
        }
        else if(temp->next[skipLevel]->data->citizenID==NULL)
        {
            //printf("NULLLL!!\n");
            temp = temp->next[skipLevel];
        }
        //if next node citizenID is bigger than what im looking for move down a lvl
        else if(atoi(temp->next[skipLevel]->data->citizenID) > atoi(word[0]))
        {        
            //printf("2: skiplevel = %d => %s\n", skipLevel, temp->next[skipLevel]->data->citizenID);
            if(skipLevel>0){
                skipLevel--;
                continue;
            }
            else 
                break;
        }
        //found
        else if( strcmp(temp->next[skipLevel]->data->citizenID, word[0])==0 && strcmp(temp->next[skipLevel]->data->virus, word[1])==0 )
        {        
            //printf("3 :skiplevel = %d => %s\n", skipLevel, temp->next[skipLevel]->data->citizenID);
            // found=1;
            //printf("found! in level %d\n", skipLevel, citizenID, temp->next[skipLevel]->data->citizenID);
            //word[2]=(char*) malloc(sizeof(char)*(strlen(temp->next[skipLevel]->data->date)+1)); 
            //strcpy(word[2], temp->next[skipLevel]->data->date);
            word[2]=temp->next[skipLevel]->data->date;
            return 1;
        }
        //if next node citizenID is smaller than what im looking for move to next node
        else if(atoi(temp->next[skipLevel]->data->citizenID) < atoi(word[0])){
            //printf("4: skiplevel = %d => %s\n", skipLevel, temp->next[skipLevel]->data->citizenID);
            temp = temp->next[skipLevel];
        }    
    }
    return 0;
    //printf("returning 0\n");
}

void get_Statistics(Sentinel** skipList, char* country, char* date1, char* date2, int filter, char** countryData, int Ncountries, int function)
{
    int d1=0, d2=INT_MAX, m1=0, m2=INT_MAX, y1=0, y2=INT_MAX;
    if(date1!=NULL && date2==NULL){
        printf("ERROR: wrong date arguments\n");
        return;
    }
    else if(date1!=NULL && date2!=NULL)
    {
        d1 = atoi(strtok (date1,"-"));
        m1 = atoi(strtok (NULL, "-"));
        y1 = atoi(strtok (NULL, "-"));
        d2 = atoi(strtok (date2,"-"));
        m2 = atoi(strtok (NULL, "-"));
        y2 = atoi(strtok (NULL, "-"));
    }
    
    Node *temp = skipList[filter]->next[0];
    if (temp==NULL){ 
        //printf("EMPTY LIST\n");
        return;
    }

    int index, age, category;//, countryFound;
    char *temporary=(char*) malloc(sizeof(char));
    int **Nvacced;
    int **Nspanvacced;
    int **Ncitizens;
    Nvacced = (int**) malloc(sizeof(int*)*Ncountries); 
    for (int i=0; i<Ncountries; i++) 
        Nvacced[i] = (int*) calloc(4, sizeof(int)); 
    Nspanvacced = (int**) malloc(sizeof(int*)*Ncountries); 
    for (int i=0; i<Ncountries; i++) 
        Nspanvacced[i] = (int*) calloc(4, sizeof(int)); 
    Ncitizens = (int**) malloc(sizeof(int*)*Ncountries); 
    for (int i=0; i<Ncountries; i++) 
        Ncitizens[i] = (int*) calloc(4, sizeof(int)); 

    if(temp->data==NULL) //skip the sentinel
        temp=temp->next[0];
    while (temp!=NULL)
    {
        //printf("%s %s %s %s %s %s %s %s\n", temp->data->citizenID, temp->data->name, temp->data->surname, temp->data->country, 
        //    temp->data->age, temp->data->virus, temp->data->vaccinated, temp->data->date);
        for(int i=0; i<Ncountries; i++)
        {
            if(!strcmp(temp->data->country, countryData[i])){
                index=i;
                // countryFound = 1;
            }
        }
        age=atoi(temp->data->age);
        if(age<20)
            category=0;
        else if(age<40)
            category=1;
        else if(age<60)
            category=2;
        else    
            category=3;
        //if(!countryFound){printf("ooof\n"); exit(-1);}
        temporary=(char*) realloc(temporary, sizeof(char)*(strlen(temp->data->date)+1));
        strcpy(temporary,temp->data->date);
        if(in_dateSpan(d1, m1, y1, temporary , d2, m2, y2)==1)
            Nspanvacced[index][category]++;
        Nvacced[index][category]++;
        Ncitizens[index][category]++;
        temp = temp->next[0];
    }

    temp = skipList[filter+1]->next[0];
    if(temp!=NULL)
        if(temp->data==NULL) //skip the sentinel
            temp=temp->next[0];
    while (temp != NULL)
    {
        //printf("%s %s %s %s %s %s %s\n", temp->data->citizenID, temp->data->name, temp->data->surname, temp->data->country, 
        //    temp->data->age, temp->data->virus, temp->data->vaccinated);
        for(int i=0; i<Ncountries; i++)
        {
            if(!strcmp(temp->data->country, countryData[i])){
                index=i;
                // countryFound = 1;
            }
        }
        age=atoi(temp->data->age);
        if(age<20)
            category=0;
        else if(age<40)
            category=1;
        else if(age<60)
            category=2;
        else    
            category=3;
        //if(!countryFound){printf("ooof\n"); exit(-1);}
        Ncitizens[index][category]++;
        temp = temp->next[0];
    }
    //populationStatus 
    if(!function)
    {
        if(country==NULL)
        {
            for(int i=0; i<Ncountries; i++)
            {
                printf("%s %d %.2f%%\n", countryData[i],
                                         Nspanvacced[i][0]+Nspanvacced[i][1]+Nspanvacced[i][2]+Nspanvacced[i][3],
                                         value((float) (Nvacced[i][0]+Nvacced[i][1]+Nvacced[i][2]+Nvacced[i][3])/
                                         (Ncitizens[i][0]+Ncitizens[i][1]+Ncitizens[i][2]+Ncitizens[i][3])*100.0) );
            }    
        }
        else{
            for(int i=0; i<Ncountries; i++)
            {
                if(!strcmp(country, countryData[i])){
                    index=i;
                    break;            
                }
            }
            /*printf("nvacced=%d nspanvacced=%d ncitizens=%d\n", Nvacced[index][0]+Nvacced[index][1]+Nvacced[index][2]+Nvacced[index][3],
                                                               Nspanvacced[index][0]+Nspanvacced[index][1]+Nspanvacced[index][2]+Nspanvacced[index][3],
                                                               Ncitizens[index][0]+Ncitizens[index][1]+Ncitizens[index][2]+Ncitizens[index][3] );*/
            printf("%d %.2f%%\n", Nvacced[index][0]+Nvacced[index][1]+Nvacced[index][2]+Nvacced[index][3],
                                  value((float)(Nspanvacced[index][0]+Nspanvacced[index][1]+Nspanvacced[index][2]+Nspanvacced[index][3])/ 
                                  (Ncitizens[index][0]+Ncitizens[index][1]+Ncitizens[index][2]+Ncitizens[index][3])*100.0) ); 
        }
    }
    //popStatusbyAge
    else
    {
        if(country==NULL)
        {
            for(int i=0; i<Ncountries; i++)
            {
                printf("%s\n0-20 %d %.2f%%\n20-40 %d %.2f%%\n40-60 %d %.2f%%\n60+ %d %.2f%%\n\n", countryData[i], 
                        Nvacced[i][0], value((float)Nspanvacced[i][0]/Ncitizens[i][0]*100.0), 
                        Nvacced[i][1], value((float)Nspanvacced[i][1]/Ncitizens[i][1]*100.0),
                        Nvacced[i][2], value((float)Nspanvacced[i][2]/Ncitizens[i][2]*100.0),
                        Nvacced[i][3], value((float)Nspanvacced[i][3]/Ncitizens[i][3]*100.0) );
            }    
        }
        else{
            for(int i=0; i<Ncountries; i++)
            {
                if(!strcmp(country, countryData[i])){
                    index=i;
                    break;            
                }
            }
            printf("%s\n0-20 %d %.2f%%\n20-40 %d %.2f%%\n40-60 %d %.2f%%\n60+ %d %.2f%%\n", countryData[index], 
                    Nvacced[index][0], value((float)Nspanvacced[index][0]/Ncitizens[index][0]*100.0), 
                    Nvacced[index][1], value((float)Nspanvacced[index][1]/Ncitizens[index][1]*100.0),
                    Nvacced[index][2], value((float)Nspanvacced[index][2]/Ncitizens[index][2]*100.0),
                    Nvacced[index][3], value((float)Nspanvacced[index][3]/Ncitizens[index][3]*100.0) );
        }
    }
    free(temporary);
    for (int i=0; i<Ncountries; i++) 
        free(Nvacced[i]); 
    free(Nvacced); 
    for (int i=0; i<Ncountries; i++) 
        free(Nspanvacced[i]); 
    free(Nspanvacced); 
    for (int i=0; i<Ncountries; i++) 
        free(Ncitizens[i]); 
    free(Ncitizens); 
}

void free_skipList(Sentinel** skipList)
{
    Node *temp, *linkedList;
    linkedList = (*skipList)->next[0];       
    while (linkedList!=NULL)
    {
        temp = linkedList;
        linkedList = linkedList->next[0];
        free(temp->next);
        free(temp);
    }
    free((*skipList)->next);
    free(*skipList);
}


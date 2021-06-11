#include <stdio.h>

typedef struct Record{
    char* citizenID;
    char* name;
    char* surname;
    char* country;
    char* age;
    char* virus;
    char* vaccinated;
    char* date;
    struct Record* next;
    int exported;
}Record;
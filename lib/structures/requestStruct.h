#include <stdio.h>

typedef struct requests{
    char *countryTo;
    char *virus;
    int nt;
    int na;
    int nr;
    char **totalTravelRequestsDate;
    char **acceptedRequestsDate;
    char **rejectedRequestsDate;
    int *totalTravelRequestsCtr;
    int *acceptedRequestsCtr;
    int *rejectedRequestsCtr;
}requests;

typedef struct reqReg{
    int n;
    requests **array;
}reqReg;
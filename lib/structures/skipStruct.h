#include <stdio.h>

typedef struct Node{
    struct Record *data;
    struct Node **next;
}Node;

typedef struct Sentinel{
    struct Record *data;
    struct Node **next;
    int NSkipLevels;
}Sentinel;

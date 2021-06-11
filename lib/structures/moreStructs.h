#include <stdio.h>
#include <arpa/inet.h>

typedef struct argvs{
    int bufferSize;
    int bloomSize;
}argvs;

typedef struct Header{
    int size;
    int type;
}Header;

typedef struct monitorDir{
    int Nsub_dirs;
    int Nread_files;
    char **sub_dir;
    char **read_files;
    int Nfiles;
    char **file;
}monitorDir;

typedef struct monitorBloomFilter{
    char *virus;
    char *bloomFilter;
}monitorBloomFilter;

typedef struct Monitor{
    pid_t pid;
    int Nsub_dirs;
    char **sub_dir;
    int NbloomFilters;
    struct monitorBloomFilter **bloomFilter;
}Monitor;

#include <stdio.h>
#define K 16

unsigned long djb2(unsigned char*);
unsigned long sdbm(unsigned char*);
unsigned long hash_i(unsigned char*, unsigned);
int my_hash(int);
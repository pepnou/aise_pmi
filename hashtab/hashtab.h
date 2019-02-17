#ifndef __HASHTAB__
#define __HASHTAB__

#define HASH_TAB_SIZE 1024

#include "../queue/queue.h"
#include "../key/key.h"

typedef elem** HashTab;

typedef struct
{
    Key key;
    long size;
    void* val;
} Data;

void init_hashtab(HashTab hash);
void* getValue(HashTab hash, Key key);
void setValue(HashTab hash, Data* data);

#endif 

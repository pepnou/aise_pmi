#ifndef __HASHTAB__
#define __HASHTAB__

#define HASH_TAB_SIZE 1024

#include <stdlib.h>

#include "../queue/queue.h"
#include "../key/key.h"

typedef Queue* HashTab;

typedef struct
{
    Key key;
    void* val;
} Data;

void init_hashtab(HashTab hash);
void* getValue(HashTab hash, Key key);
void setValue(HashTab hash, Key key, void* val, void (*freeVal)(void*));
void freeData(Data* data, void (*freeVal)(void*));
void freeHash(HashTab hash, void (*freeVal)(void*));

#endif 

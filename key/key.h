#ifndef __KEY__
#define __KEY__

#define KEY_SIZE 256

#include <stdlib.h>
#include <stdint.h>

typedef uint64_t* Key;

void init_key(Key* key);
int modulo(Key key, int N);
int isKeyEqual(void* a, void* b);
void copy_key(Key src, Key dest);
void freeKey(Key key);


#endif

#ifndef __KEY__
#define __KEY__

#define KEY_SIZE 256

typedef uint64_t* Key;

void init_key(Key key);
int modulo(Key key, int N);
int isEqual(Key key1, Key key2);


#endif

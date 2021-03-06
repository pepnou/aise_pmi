#include "key.h"
#include <stdio.h>

void init_key(Key* key)
{
    *key = malloc(KEY_SIZE / 8);
    if(*key == NULL)
    {
        perror("malloc");
        exit(1);
    }
}

// (2^KEY_SIZE) % N doit etre egal a 0
// sinon le resultat sera faux
int modulo(Key key, int N)
{
    int res = 0;

    for(int i = 0; i < KEY_SIZE / 64; i++)
        res += key[i] % N;

    return res % N;
}

int isKeyEqual(void* a, void* b)
{
    Key key1 = (Key)a;
    Key key2 = (Key)b;
    int res = 1;

    for(int i = 0; i < KEY_SIZE / 64 && res; i++)
        res = res && (key1[i] == key2[i]);
    
    return res;
}

void copy_key(Key src, Key dest)
{
    for(int i = 0; i < KEY_SIZE / 64; i++)
        dest[i] = src[i];
}


void freeKey(Key key)
{
    free(key);
}

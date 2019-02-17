#include "key.h"

void init_key(Key key)
{
    key = malloc(KEY_SIZE / 8);
}

// (2^KEY_SIZE) % N doit etre egal a 0
// sinon le resultat sera faux
int modulo(Key key, int N)
{
    int res = 0;

    for(int i = 0; i < KEY_SIZE / 64; i++)
        res += key[i] % N;
    
    return res;
}

int isKeyEqual(void* a, void* b)
{
    Key key1 = (Key)a;
    Key key2 = (Key)b;
    int res = 1;

    for(int i = 0; i < KEY_SIZE / 64; i++)
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

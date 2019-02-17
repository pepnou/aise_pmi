#include "hashtab.h"


void init_hashtab(HashTab hash)
{
    hash = malloc(HASH_TAB_SIZE*sizeof(elem*));
    for(int i = 0; i < HASH_TAB_SIZE; i++)
        hash[i] = NULL;
}

void* getValue(HashTab hash, Key key)
{
    return findInQueue(hash[modulo(key, HASH_TAB_SIZE)], key, isKeyEqual);
}

void setValue(HashTab hash, Key key, void* val, void (*freeVal)(void*))
{
    Data* previous = getValue(hash, key);
    if(previous)
    {
        freeVal(previous->val);
        previous->val = val;
    }
    else
    {
        Data* new_data = malloc(sizeof(Data));
        init_key(new_data->key);
        copy_key(key, new_data->key);
        new_data->val = val;
        ajout_deb(&(hash[modulo(key, HASH_TAB_SIZE)]), new_data);
    }
}

void freeData(Data* data, void (*freeVal)(void*))
{
    freeKey(data->key);
    freeVal(data->val);
    free(data);

}

void freeHash(HashTab hash, void (*freeVal)(void*))
{
    for(int i = 0; i < HASH_TAB_SIZE; i++)
        freeQueue(hash[i], freeData);

    free(hash);
}


#include "hashtab.h"
#include <stdio.h>


void init_hashtab(HashTab* hash)
{
    *hash = malloc(HASH_TAB_SIZE*sizeof(Queue));
    if(!(*hash))
    {
        perror("malloc");
        exit(1);
    }

    for(int i = 0; i < HASH_TAB_SIZE; i++)
        (*hash)[i] = NULL;
}

void* getValue(HashTab hash, Key key)
{
    return findInQueue(hash[modulo(key, HASH_TAB_SIZE)], key, isKeyEqual);
}

void* setValue(HashTab hash, Key key, void* val)
{
    void* previous_val = NULL;

    Data* previous = getValue(hash, key);
    if(previous)
    {
        previous_val = previous->val;
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

    return previous_val;
}


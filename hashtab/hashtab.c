#include "hashtab.h"
#include <stdio.h>
#include <stdlib.h>


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

int isKeyDataEqual(void* k, void* d)
{
    Key key = (Key) k;
    Data* data = (Data*) d;
    
    return isKeyEqual(key, data->key);
}

void* getValue(HashTab hash, Key key)
{
    fprintf(stderr, "get hash[%d]\n", modulo(key, HASH_TAB_SIZE));
    Data* data = (Data*)findInQueue(hash[modulo(key, HASH_TAB_SIZE)], (void*)key, isKeyDataEqual);
    if(data)
        return data->val;
    else
        return NULL;
}

void* setValue(HashTab* hash, Key key, void* val)
{
    void* previous_val = NULL;

    Data* previous = getValue(*hash, key);
    if(previous)
    {
        previous_val = previous->val;
        previous->val = val;
    }
    else
    {
        Data* new_data = malloc(sizeof(Data));
        if(new_data == NULL)
        {
            perror("malloc");
            exit(1);
        }
        init_key(&(new_data->key));
        copy_key(key, new_data->key);
        new_data->val = val;
    
        fprintf(stderr, "set hash[%d]\n", modulo(key, HASH_TAB_SIZE));
        ajout_deb(&((*hash)[modulo(key, HASH_TAB_SIZE)]), (void*)new_data);
    }

    return previous_val;
}


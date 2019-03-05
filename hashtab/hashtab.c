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

void setValue(HashTab hash, Key key, void* val, Queue freeVal)
{
    Data* previous = getValue(hash, key);
    if(previous)
    {
        if(freeVal == NULL)
            free(previous->val);
        else
        {
            void (*f)(void*, Queue) = (void (*)(void*, Queue))(freeVal->val);
            f(previous->val, freeVal->suiv);
        }
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

void freeData(Data* data, Queue freeVal)
{
    
    freeKey(data->key);
    
    if(freeVal == NULL)
        free(data->val);
    else
    {
        void (*f)(void*, Queue) = (void (*)(void*, Queue))(freeVal->val);
        f(data->val, freeVal->suiv);
    }
    
    free(data);
}

void freeHash(HashTab hash, Queue freeVal)
{
   Queue nfreeVal = NULL;
   ajout_deb(&nfreeVal, (void*)freeData);
   nfreeVal->suiv = freeVal;

    for(int i = 0; i < HASH_TAB_SIZE; i++)
        freeQueue(&hash[i], nfreeVal);

    free(nfreeVal);
    free(hash);
}


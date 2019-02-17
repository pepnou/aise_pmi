#include "hashtab.h"


void init_hashtab(HashTab hash)
{
    hash = malloc(HASH_TAB_SIZE*sizeof(elem*));
    for(int i = 0; i < HASH_TAB_SIZE; i++)
        hash[i] = NULL;
}

Data* getValue(HashTab hash, Key key)
{
    elem *list = hash[modulo(key, HASH_TAB_SIZE)];
    Data *data = NULL;

    while(list)
    {
        if(isEqual(((Data*)list->val)->key, key))
        {
            data = (Data*)list->val;
            break;
        }

        list = list->suiv;
    }
    return data;
}

void setValue(HashTab hash, Data* data)
{
    Data* previous = getValue(hash, data->key);
    if(previous)
    {
        free(previous->val);
        previous->size = data->size;
        previous->val = data->val;

    }
    else
    {
        ajout_deb(&(hash[modulo(key, HASH_TAB_SIZE)]), data);
    }
}

void freeData(Data* data)
{
    freeKey(data->key);
    free(data->val);
    free(data);

}

void freeHash(HashTab hash)
{
    for(int i = 0; i < HASH_TAB_SIZE; i++)
        freeQueue(hash[i], freeData);

    free(hash);
}


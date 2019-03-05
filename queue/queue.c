#include "queue.h"
#include <stdlib.h>


void ajout_deb(Queue* list, void* val)
{
	Queue new_elem = malloc(sizeof(elem));
	new_elem->val = val;
	new_elem->suiv = *list;
	*list = new_elem;
}

void* findInQueue(Queue list, void* val, int (*isEqual)(void*,void*))
{
    Queue tmp = list;
    while(tmp)
    {
        if(isEqual(val, tmp->val))
            return tmp->val;
    }
    return NULL;
}

void freeQueue(Queue* list, Queue freeVal)
{
    void (*f)(void) = (void (*)(void))(freeVal->val);

    Queue tmp1 = *list;
    Queue tmp2;
    while(tmp1)
    {
	tmp2 = tmp1->suiv;

        if(f == free)
            f(tmp1->val);
        else
            f(tmp1->val, freeVal->suiv);
	
        free(tmp1);;
	tmp1 = tmp2;
    }
    *list = NULL;
}

void fakefreeQueue(Queue* list)
{
    Queue tmp1 = *list;
    Queue tmp2;
    while(tmp1)
    {
	tmp2 = tmp1-suiv;	
        free(tmp1);;
	tmp1 = tmp2;
    }
    *list = NULL;
}

void supprElem(Queue* list, int i, Queue freeVal)
{
    Queue tmp;
    Queue tmp2;
    if(i == 0) // cas particulier de la tete de liste
    {
        tmp = *list;

        if(freeVal == NULL)
            free(tmp->val);
        else
        {
            void (*f)(void*, Queue) = (void (*)(void*, Queue))(freeVal->val);
            f(tmp->val, freeVal->suiv);
        }
        
        *list = tmp->suiv;
        free(tmp);
    }
    else
    {
        for(int j = 0; j < i-1; j++) //on se deplace au parent de l'élément a supprimer
            tmp = tmp->suiv;

        if(freeVal == NULL)
            free(tmp->suiv->val);
        else
        {
            void (*f)(void*, Queue) = (void (*)(void*, Queue))(freeVal->val);
            f(tmp->suiv->val, freeVal->suiv);
        }

        tmp2 = tmp->suiv->suiv;
        free(tmp->suiv);
        tmp->suiv = tmp2;
    }
}

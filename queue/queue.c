#include "queue.h"
#include <stdlib.h>
#include <stdio.h>


void ajout_deb(Queue* list, void* val)
{
	Queue new_elem = malloc(sizeof(elem));
        if(new_elem == NULL)
        {
            perror("malloc");
            exit(1);
        }
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
        tmp = tmp->suiv;
    }
    return NULL;
}

void* supprElem(Queue* list, int i)
{
    void* retval;

    Queue tmp = *list;
    Queue tmp2;
    if(i == 0) // cas particulier de la tete de liste
    {
        retval = tmp->val;
        *list = tmp->suiv;
        free(tmp);
    }
    else
    {
        for(int j = 0; j < i-1; j++) //on se deplace au parent de l'élément a supprimer
            tmp = tmp->suiv;

        retval = tmp->suiv->val;

        tmp2 = tmp->suiv->suiv;
        free(tmp->suiv);
        tmp->suiv = tmp2;
    }

    return retval;
}

#include "queue.h"


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

void freeQueue(Queue* list, void (*freeVal)(void*))
{
    Queue tmp1 = *list;
    Queue tmp2;
    while(tmp1)
    {
	tmp2 = tmp1->suiv;
        freeVal(tmp1->val);
	free(tmp1);;
	tmp1 = tmp2;
    }
    *list = NULL;
}

void supprElem(Queue* list, int i, void (*freeVal)(void*))
{
    Queue tmp;
    Queue tmp2;
    if(i == 0) // cas particulier de la tete de liste
    {
        tmp = *list;
        freeVal(tmp->val);
        *list = tmp->suiv;
        free(tmp);
    }
    else
    {
        for(int j = 0; j < i-1; j++) //on se deplace au parent de l'élément a supprimer
            tmp = tmp->suiv;

        freeVal(tmp->suiv->val);
        tmp2 = tmp->suiv->suiv;
        free(tmp->suiv);
        tmp->suiv = tmp2;
    }
}

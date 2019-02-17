#include "queue.h"


void ajout_deb(elem** liste, void* val)
{
	elem* new_elem = malloc(sizeof(elem));
	new_elem->val = val;
	new_elem->suiv = *liste;
	*liste = new_elem;
}

void lib_mem_list(elem** liste)
{
	elem *tmp1 = *liste, *tmp2;
	while(tmp1)
	{
		tmp2 = tmp1->suiv;
		free(tmp1);
		tmp1 = tmp2;
	}
	*liste = NULL;
}

// TODO
void* findInQueue(elem* liste, void* val, int (*isEqual)(void*,void*))
{
    
}

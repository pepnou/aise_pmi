#ifndef __QUEUE__
#define __QUEUE__


struct elem
{
	void* val;
	struct elem* suiv;
};
typedef struct elem elem;

void ajout_deb(elem** liste, void* val);
void lib_mem_list(elem** liste);
void* findInQueue(elem* liste, void* val, int (*isEqual)(void*,void*));


#endif

#ifndef __QUEUE__
#define __QUEUE__


struct elem
{
	void* val;
	struct elem* suiv;
};
typedef struct elem elem;

typedef elem* Queue;

void ajout_deb(Queue* list, void* val);
void* findInQueue(Queue list, void* val, int (*isEqual)(void*,void*));
/*void freeQueue(Queue* list, Queue freeVal);
void fakefreeQueue(Queue* list);*/
void* supprElem(Queue* list, int i);


#endif

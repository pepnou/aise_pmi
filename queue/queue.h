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
void freeQueue(Queue list, void (*freeVal)(void*));
void supprElem(Queue* list, int i, void (*freeVal)(void*));


#endif

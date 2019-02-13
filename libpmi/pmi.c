#include "pmi.h"
#include <stdlib.h>

typedef struct {
	int size;
	int rank;
	long jobid;
} Info;

Info info;


/* Initialise la bibliothèque client PMI */
int PMI_Init()
{
	info.size = atoi(getenv("PMI_PROCESS_COUNT"));
	info.rank = atoi(getenv("PMI_RANK"));
	info.jobid = atol(getenv("PMI_JOB_ID"));
	return PMI_SUCCESS;
}

/* Libère la bibliothèque client PMI */
int PMI_Finalize(void)
{
	return PMI_SUCCESS;
}


/* Donne le nombre de processus faisant partie du JOB */
int PMI_Get_size(int *size)
{
	*size = info.size;
	return PMI_SUCCESS;
}


/* Donne le rang du processus courant */
int PMI_Get_rank(int *rank)
{
	*rank = info.rank;
	return PMI_SUCCESS;
}

/* Donne un ID unique pour le job courant */
int PMI_Get_job(int *jobid)
{
	*jobid = info.jobid;
	return PMI_SUCCESS;
}



/* Effectue une barrière synchronisante entre les processus */
int PMI_Barrier(void)
{
	return PMI_SUCCESS;
}

/* Ajoute une clef et une valeur dans le stockage de la PMI */
int PMI_KVS_Put( char key[],  char value[])
{
	return PMI_SUCCESS;
}


/* Lit une clef depuis le stockage de la PMI */
int PMI_KVS_Get( char key[], char value[], int length)
{
	return PMI_SUCCESS;
}



#ifndef PMI_H
#define PMI_H

/* Valeur renvoyée en cas de succès */
#define PMI_SUCCESS 0
/* Valeur renvoyée en cas d’erreur */
#define PMI_ERROR 1
/* Valeur renvoyée si une clef n’est pas trouvée par PMI_KVS_Get */
#define PMI_NO_KEY 2

/* Initialise la bibliothèque client PMI */
int PMI_Init();
/* Libère la bibliothèque client PMI */
int PMI_Finalize(void);
/* Donne le nombre de processus faisant partie du JOB */
int PMI_Get_size(int *size);
/* Donne le rang du processus courant */
int PMI_Get_rank(int *rank);
/* Donne un ID unique pour le job courant */
int PMI_Get_job(int *jobid);

/* Effectue une barrière synchronisante entre les processus */
int PMI_Barrier(void);

/* Définit la longueur maximale d’une CLEF et d’une VALEUR */
#define PMI_STRING_LEN 256

/* Ajoute une clef et une valeur dans le stockage de la PMI */
int PMI_KVS_Put( char key[],  void* val, long size);

/* Lit une clef depuis le stockage de la PMI */
int PMI_KVS_Get( char key[], void* val, long *size);

/* Fait le demande a la PMI de recuperer une valeur associé a une clé */
void PMI_KVS_Get_rqst( char key[]);

/* Attend la réponse de la PMI apres l'appel a PMI_KVS_Get_rqst */
int PMI_KVS_Get_wait( void* val, long *size);

/* Definie le début d'une section contenant des appels aux fonctions PMI_KVS_Put, PMI_KVS_Get_rqst et PMI_KVS_wait */
void PMI_Lock();

/* Definie la fin d'une section contenant des appels aux fonctions PMI_KVS_Put, PMI_KVS_Get_rqst et PMI_KVS_wait */
void PMI_Unlock();

/*

*/

#endif /* PMI_H */

#include "pmi.h"
#include "../safeIO/safeIO.h"
#include "../key/key.h"
#include "../sha256/sha256.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
	long size;
	long rank;
	long jobid;
	int fd;
        sha256_context sha;
} Info;

Info info;

/* Initialise la bibliothèque client PMI */
int PMI_Init()
{
        sha256_starts(&(info.sha));

	info.size = atol(getenv("PMI_PROCESS_COUNT"));
	info.rank = atol(getenv("PMI_RANK"));
	info.jobid = atol(getenv("PMI_JOB_ID"));

	char *ip, *port;
	ip = strtok(getenv("PMI_SERVER_ADDR"), ":");
	port = strtok(NULL, ":");


	/* Liste des alternatives
	 * pour se connecter */
	struct addrinfo *res = NULL;
	/* Configuration souhaitée */
	struct addrinfo hints;
	/* Toujours mettre hints a 0 */
	memset(&hints, 0, sizeof(hints));
	/* On veut IPV4 ou IPV6, respectivement AF_INET ou AF_INET6 */
	hints.ai_family = AF_INET;
	/* On veut faire du TCP */
	hints.ai_socktype = SOCK_STREAM;

	/* On lance la résolution DNS
	 * argv[1] : nom de l'hôte
	 * argv[2] : port (exemple 80 pour http)
	 * */
	int ret = getaddrinfo(ip, port,
			&hints,
			&res);

	if( ret < 0)
	{
		herror("getaddrinfo");
		return 1;
	}

	struct addrinfo *tmp;
	info.fd = -1;
	int connected = 0;

	/* On parcours les alternative recues */
	for (tmp = res; tmp != NULL; tmp = tmp->ai_next) {
		/* On crée un socket en suivant la configuration
		 * renvoyéee par getaddrinfo */
		info.fd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);

		if( info.fd < 0) {
			perror("sock");
			continue;
		}

		/* On tente de le connecter à l'adresse renvoyée par
		 * getaddrinfo et configurée par argv[1] et argv[2] */
		int ret = connect( info.fd, tmp->ai_addr, tmp->ai_addrlen);

		if( ret < 0 ) {
			close(info.fd);
			perror("connect");
			continue;
		}
		connected = 1;
		break;
	}

	if(!connected)
	{
		fprintf(stderr, "Failed to connect to %s:%s\n",ip, port);
		return 1;	
	}

	fprintf(stderr, "Connected to %s:%s\n",ip, port);

	/* Si nous sommes là le socket est connecté
	 * avec succes on peut lire et ecrire dedans */

	safe_write(info.fd, (char*)(&(info.jobid)), sizeof(long), 0);
	safe_write(info.fd, (char*)(&(info.size)), sizeof(long), 0);

	return PMI_SUCCESS;
}

/* Libère la bibliothèque client PMI */
int PMI_Finalize(void)
{
	long instruction = -1;
	safe_write(info.fd, (char*)&instruction, sizeof(long), 0);
	close(info.fd);
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
int PMI_Barrier()
{
	long instruction = -2;
	safe_write(info.fd, (char*)&instruction, sizeof(long), 0);
	fprintf(stderr, "j'ai ecirt\n");
	char c;
	//safe_read(info.fd, &c, 1, 0);
	fprintf(stderr, "j'ai lu\n");

    return PMI_SUCCESS;
}

/* Ajoute une clef et une valeur dans le stockage de la PMI */
int PMI_KVS_Put( char key[],  void* val, long size)
{
    char* buf = (char*)val;

    Key hashed_key;
    init_key(hashed_key);

    sha256_update(&(info.sha), (unsigned char*)key, size);
    sha256_finish(&(info.sha), (unsigned char*)hashed_key);

    safe_write(info.fd, (char*)&size, sizeof(long), 0);
    safe_write(info.fd, (char*)&hashed_key, KEY_SIZE / 8, 0);
    safe_write(info.fd, buf, size, 0);
    
    freeKey(hashed_key);
    return PMI_SUCCESS;
}


/* Lit une clef depuis le stockage de la PMI */
int PMI_KVS_Get( char key[], void* val, long size)
{
    char* buf = (char*)val;

    Key hashed_key;
    init_key(hashed_key);

    sha256_update(&(info.sha), (unsigned char*)key, size);
    sha256_finish(&(info.sha), (unsigned char*)hashed_key);

    safe_write(info.fd, (char*)&size, sizeof(long), 0);
    safe_write(info.fd, (char*)&hashed_key, KEY_SIZE / 8, 0);

    safe_read(info.fd, (char*)&size, sizeof(long), 0);

    freeKey(hashed_key);

    if(size)
    {
        safe_read(info.fd, buf, size, 0);
        return PMI_SUCCESS;
    }
    else
        return PMI_NO_KEY;
}


#include "pmi.h"
#include "../safeIO/safeIO.h"
#include "../key/key.h"
#include "../sha256/sha256.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

typedef struct {
	long size;
	long rank;
	long jobid;
	Comm comm;
        sha256_context sha;
} Info;

Info info;

/* Initialise la bibliothèque client PMI */
int PMI_Init()
{
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
	info.comm.fd = -1;
	int connected = 0;

	/* On parcours les alternative recues */
	for (tmp = res; tmp != NULL; tmp = tmp->ai_next) {
		/* On crée un socket en suivant la configuration
		 * renvoyéee par getaddrinfo */
		info.comm.fd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);

		if( info.comm.fd < 0) {
			perror("sock");
			continue;
		}

		/* On tente de le connecter à l'adresse renvoyée par
		 * getaddrinfo et configurée par argv[1] et argv[2] */
		int ret = connect( info.comm.fd, tmp->ai_addr, tmp->ai_addrlen);

		if( ret < 0 ) {
			close(info.comm.fd);
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

	safe_write_fd(info.comm.fd, (char*)(&(info.jobid)), sizeof(long), 0);
	safe_write_fd(info.comm.fd, (char*)(&(info.size)), sizeof(long), 0);
        
        char comm_type;

        if(!strncmp(ip, "127.0.0.", 8)) //shm
        {
            char* file_name = malloc(1024*sizeof(char));
            int mmap_fd = 0;
                                
            sprintf(file_name, "./map/%ld_%ld_1", info.jobid, info.rank);
            mmap_fd = open(file_name, O_CREAT | O_RDWR,0666);
            ftruncate(mmap_fd, SHM_SIZE);
            info.comm.in = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, 0);
            if(info.comm.in == MAP_FAILED)
            {
                perror("mmap");
                exit(1);
            }
            memset(info.comm.in, 0, SHM_SIZE);
            msync(info.comm.in, SHM_SIZE, MS_SYNC | MS_INVALIDATE);

            sprintf(file_name, "./map/%ld_%ld_0", info.jobid, info.rank);
            mmap_fd = open(file_name, O_CREAT | O_RDWR,0666);
            ftruncate(mmap_fd, SHM_SIZE);
            info.comm.out = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, 0);
            if(info.comm.out == MAP_FAILED)
            {
                perror("mmap");
                exit(1);
            }
            memset(info.comm.out, 0, SHM_SIZE);
            msync(info.comm.out, SHM_SIZE, MS_SYNC | MS_INVALIDATE);

            info.comm.in_offset = 0;
            info.comm.out_offset = 0;

            comm_type = 2;
            safe_write_fd(info.comm.fd, &comm_type, 1, 0);
            safe_write_fd(info.comm.fd, (char*)&info.rank, sizeof(long), 0);

            close(info.comm.fd);
            info.comm.fd = -1;
        }
        else //socket
        {
            comm_type = 1;
            safe_write_fd(info.comm.fd, &comm_type, 1, 0);
        }

        PMI_Barrier();

	return PMI_SUCCESS;
}

/* Libère la bibliothèque client PMI */
int PMI_Finalize(void)
{   
        PMI_Barrier();
	long instruction = -1;
	safe_write(&(info.comm), (char*)&instruction, sizeof(long), 0);
	//close(info.fd);
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
	safe_write(&(info.comm), (char*)&instruction, sizeof(long), 0);
	long c;
	safe_read(&(info.comm), (char*)&c, sizeof(long), 0);

    return PMI_SUCCESS;
}

/* Ajoute une clef et une valeur dans le stockage de la PMI */
int PMI_KVS_Put( char key[],  void* val, long size)
{
    char* buf = (char*)val;

    Key hashed_key;
    init_key(&hashed_key);

    sha256_starts(&(info.sha));
    sha256_update(&(info.sha), (unsigned char*)key, strlen(key));
    sha256_finish(&(info.sha), (unsigned char*)hashed_key);
    
    safe_write(&(info.comm), (char*)&size, sizeof(long), 0);
    safe_write(&(info.comm), (char*)hashed_key, KEY_SIZE / 8, 0);
    safe_write(&(info.comm), buf, size, 0);
    
    freeKey(hashed_key);
    return PMI_SUCCESS;
}


/* Lit une clef depuis le stockage de la PMI */
int PMI_KVS_Get( char key[], void* val, long size)
{
    char* buf = (char*)val;
    
    size = -3;

    Key hashed_key;
    init_key(&hashed_key);

    sha256_starts(&(info.sha));
    sha256_update(&(info.sha), (unsigned char*)key, strlen(key));
    sha256_finish(&(info.sha), (unsigned char*)hashed_key);
    
    safe_write(&(info.comm), (char*)&size, sizeof(long), 0);
    safe_write(&(info.comm), (char*)hashed_key, KEY_SIZE / 8, 0);

    safe_read(&(info.comm), (char*)&size, sizeof(long), 0);

    freeKey(hashed_key);

    if(size)
    {
        safe_read(&(info.comm), buf, size, 0);
        return PMI_SUCCESS;
    }
    else
        return PMI_NO_KEY;
}


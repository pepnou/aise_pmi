#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

struct elem
{
	void* val;
	struct elem* suiv;
};
typedef struct elem elem;

typedef struct 
{
	long jobid;
	long barrier;
	long nb_processes;
	elem* processes;
} Job;

void ajout_deb(elem** liste, void* val)
{
	elem* new_elem = malloc(sizeof(elem));
	new_elem->val = val;
	new_elem->suiv = *liste;
	*liste = new_elem;
}

void lib_mem(elem** liste)
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

int guard(int n, char* error)
{
	if(n < 0)
	{
		perror(error);
		herror(error);
		exit(1);
	}
	return n;
}

void safe_read(int fd, int size, void* buf)
{
	int red = 0;
	while(red != size)
	{
		red = read(fd, buf + red, size - red);
		if(red == -1)
		{
			perror("safe_read");
			exit(1);
		}
	}
}

void safe_write(int fd, int size, void* buf)
{
	int wrote = 0;
	while(wrote != size)
	{
		wrote = write(fd, buf + wrote, size - wrote);
		if(wrote == -1)
		{
			perror("safe_write");
			exit(1);
		}
	}
}


int main( int argc, char ** argv )
{
	/* Valeur de retour de getaddrinfo
	 * avec les alternatives pour configurer
	 * le serveur en fonction des hints */
	struct addrinfo *res = NULL;

	/* Ces paramètres définissent comment
	 * le serveur doit être configuré */
	struct addrinfo hints;
	/* Toujours mettre la structure a 0 */
	memset(&hints, 0, sizeof(hints)); 
	/* Peu importe le protocole IPV4 (AF_INET)
	 * ou IPV6 (AF_INET6) */
	hints.ai_family = AF_UNSPEC;
	/* Nous voulons un socket TCP */
	hints.ai_socktype = SOCK_STREAM;
	/* Ce paramètre permet d'écouter 
	 * hors de la machine locale */
	hints.ai_flags = AI_PASSIVE;

	/* Ce programme attend un paramètre qui est le port */
	if(argc != 2)
		return 1;

	fprintf(stderr, "server starting on port %d\n", atoi(argv[1]));

	/* Ici getaddrinfo permet de générer les 
	 * configurations demandées */
	int ret = getaddrinfo(NULL, argv[1],
								&hints,
								&res);
	if(ret != 0)
	{
		herror("getaddrinfo");
		exit(1);
	}

	struct addrinfo *tmp;
	int listen_sock = -1;
	int binded = 0;

	/* Nous allons parcourir les différentes
	 * configurations pour trouver une qui marche
	 * ces configurations sont définies par les hints */

	for (tmp = res; tmp != NULL; tmp = tmp->ai_next)
	{

		/* On crée un socket */
		listen_sock = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);

		if( listen_sock < 0)
		{
			perror("sock");
			continue;
		}

		/* On attache ce socket à un port donné (argument passé à getaddr) */
		ret = bind( listen_sock, tmp->ai_addr, tmp->ai_addrlen);

		if( ret < 0 ) {
			close(listen_sock);
			perror("bind");
			continue;
		}
		binded = 1;
		break;
	}

	if(!binded)
	{
		fprintf(stderr, "Failed to bind on 0.0.0.0:%s\n", argv[1]);
		return 1;	
	}

	/* On passe listen_sock en non-blocant */
	int flags = fcntl(listen_sock, F_GETFL);
	if(flags == -1)
	{
		perror("get flags");
		exit(1);
	}
  	ret = fcntl(listen_sock, F_SETFL, flags | O_NONBLOCK);
  	if(ret == -1)
	{
		perror("get flags");
		exit(1);
	}

	/* On commence a ecouter */
	guard(listen(listen_sock, 100), "listen");


	/* On va maintenant accepter une connexion */

	/* Ceci sera remplis par
	 * accept et décrit le client (ip port)*/
	struct sockaddr client_info;
	socklen_t addr_len;



	char *buf = malloc(512 * sizeof(char));
	elem *jobs = NULL;
	elem *temp = NULL;
	elem *temp2 = NULL;
	long jobid, instruction, nb_processes;

	fprintf(stderr, "server online\n");
	int client_socket;

	while(1)
	{
		/* On accepte tous les clients et on récupére leur nouveau FD */
		while((client_socket = accept(listen_sock, &client_info, &addr_len)) != -1)
		{
			jobid = nb_processes = 0;
			safe_read(client_socket, sizeof(long), (void*)&jobid);
			safe_read(client_socket, sizeof(long), (void*)&nb_processes);

			fprintf(stderr, "new connection for job %ld composed of %ld processes\n", jobid, nb_processes);

			temp = jobs;
			while(temp)
			{
				if(((Job*)(temp->val))->jobid == jobid)
				{
					ajout_deb(&(((Job*)(temp->val))->processes), (void*)malloc(sizeof(int)));
					*((int*)((((Job*)(temp->val))->processes)->val)) = client_socket;
					//((Job*)(jobs->val))->nb_processes += 1;

					jobid = -1;
				}
				temp = temp->suiv;
			}

			if(jobid != -1)
			{
				ajout_deb(&jobs, (void*)malloc(sizeof(Job)));
				((Job*)(jobs->val))->processes = NULL;
				((Job*)(jobs->val))->nb_processes = nb_processes;
				((Job*)(jobs->val))->barrier = 0;

				ajout_deb(&(((Job*)(jobs->val))->processes), (void*)malloc(sizeof(int)));
				*((int*)((((Job*)(jobs->val))->processes)->val)) = client_socket;
			}
		}

		if(errno == EWOULDBLOCK || errno == EAGAIN) //traitement
		{
			//fprintf(stderr, "No pending connection, processing connected processes");
			temp = jobs;
			while(temp)
			{
				temp2 = ((Job*)(temp->val))->processes;
				while(temp2)
				{
					int red = guard( read(*(int*)(temp2->val), (void*)&instruction, sizeof(long)), "write");
					if(red)
					{
						safe_read(*(int*)(temp2->val), sizeof(long) - red, ((void*)&instruction)+red);
						fprintf(stderr, "%ld\n", instruction);
					}
					temp2 = temp2->suiv;
				}
				temp = temp->suiv;
			}
		}
		else
		{
			perror("accept");
			exit(1);
		}
	}


	/* On ferme le serveur */
	close(listen_sock);

	return 0;
}
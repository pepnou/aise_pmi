#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
		exit(1);
	}
	return n;
}

void safe_read(int fd, int size, char* buf)
{
	int red = 0;
	while(red != size)
	{
		red = read(fd, &buf[red], size - red);
	}
}

void safe_write(int fd, int size, char* buf)
{
	int wrote = 0;
	while(wrote != size)
	{
		wrote = write(fd, &buf[wrote], size - wrote);
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
	hints.ai_socktype = SOCK_STREAM | SOCK_NONBLOCK;
	/* Ce paramètre permet d'écouter 
	 * hors de la machine locale */
	hints.ai_flags = AI_PASSIVE;

	/* Ce programme attend un paramètre qui est le port */
	if(argc != 2)
		return 1;

	/* Ici getaddrinfo permet de générer les 
	 * configurations demandées */
	int ret = guard(getaddrinfo(NULL, argv[1],
								&hints,
								&res),
					"getaddrinfo");

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

		if( listen_sock < 0) {
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

	/* On commence a ecouter */
	ret = guard(listen(listen_sock, 2), "listen");


	/* On va maintenant accepter une connexion */

	/* Ceci sera remplis par
	 * accept et décrit le client (ip port)*/
	struct sockaddr client_info;
	socklen_t addr_len;



	char *buf = malloc(512 * sizeof(char));
	elem *jobs = NULL;
	elem *temp = NULL;
	double jobid;


	while(1)
	{
		/* On accepte un client et on récupére un nouveau FD */
		int client_socket = accept(listen_sock, &client_info, &addr_len);
		safe_read(client_socket, sizeof(long), buf);
		jobid = *buf;


		temp = jobs;
		while(temp)
		{
			if(((*Job)(temp->val))->jobid == jobid)
			{
				ajout_deb(&(((*Job)(temp->val))->processes), (void*)malloc(sizeof(int)));
				(int)(((*Job)(temp->val))->val) = client_socket;

				jobid = -1;
			}
		}
		if(jobid != -1)
		{
			ajout_deb(&jobs, (void*)malloc(sizeof(Job)));
			((*Job)(jobs->val))->processes = NULL;
		}



		fprintf(stderr,"After accept\n");

		if( client_socket < 0)
		{
			perror("accept");
			return 1;
		}

		if (client_socket == -1 && errno != EWOULDBLOCK && errno != EAGAIN)
		{
			perror("accept");
			exit(1);
		}
		else
		{
			//TRAITEMENT
		}

	}


	/* On ferme le serveur */
	close(listen_sock);

	return 0;
}
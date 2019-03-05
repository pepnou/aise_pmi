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

#include "../queue/queue.h"
#include "../hashtab/hashtab.h"
#include "../key/key.h"
#include "../safeIO/safeIO.h"


// TODO : les suppression a refaire

typedef struct
{
    long size;
    void* val;
} Message;


typedef struct 
{
	long jobid;
	long barrier;
	long nb_processes;
	Queue processes;
        HashTab hash_tab;
} Job;

void freeMsg(void* msg, Queue freeVal)
{
    Message* m = (Message*) msg;
    free(m->val);
}

void freeJob(void* job, Queue freeVal)
{
    Job* j = (Job*)job;

    freeQueue(&(j->processes), NULL);

    Queue nfreeVal = NULL;
    ajout_deb(&nfreeVal, (void*)freeMsg);
    
    freeHash(j->hash_tab, nfreeVal);

    fakefreeQueue(&nfreeVal);
}

void traitement(Job* job, int fd, int i, long instruction )
{
    Key key; 
    long size = 0;
    Message* msg;

    switch(instruction)
    {
        case -1: //processus end 
        {
            //supprElem(&(job->processes), i, NULL);
            job->nb_processes -= 1;
            break;
        }
        case 0: //get key
        {
            safe_read(fd, KEY_SIZE / 8, &key);
            msg = getValue(job->hash_tab, key);

            if(msg)
            {
                safe_write(fd, sizeof(long), &(msg->size));
                safe_write(fd, msg->size, (void*)(msg->val));
            }
            else
                safe_write(fd, sizeof(long), &size);
            break;
        }
        default: //set key of size 'instruction'
        {
            safe_read(fd, sizeof(long), &key);
            msg = malloc(sizeof(msg));
            msg->size = instruction;
            msg->val = malloc(sizeof(msg->size));

            Queue tmp = NULL;
            ajout_deb(&tmp, (void*)freeMsg);
            setValue(job->hash_tab, key, (void*)msg, tmp);
            fakefreeQueue(&tmp);
            break;
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

	if( ret < 0 )
        {
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
    ret = listen(listen_sock, 100);
    if(ret < 0)
    {
        perror("listen");
        exit(1);
    }


    /* On va maintenant accepter une connexion */

    /* Ceci sera remplis par
     * accept et décrit le client (ip port)*/
    struct sockaddr client_info;
    socklen_t addr_len;



    Queue jobs = NULL;
    Queue temp = NULL;
    Queue temp2 = NULL;
    long jobid, instruction, nb_processes;
    Job* job;

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
                job = (Job*)(temp->val);
		if(job->jobid == jobid)
		{
		    ajout_deb(&(job->processes), (void*)malloc(sizeof(int)));
		    *((int*)((job->processes)->val)) = client_socket;

		    jobid = -1;
                    break;
		}
		temp = temp->suiv;
	    }

	    if(jobid != -1)
	    {
                job = malloc(sizeof(Job));
                job->processes = NULL;
                job->nb_processes = nb_processes;
                job->barrier = 0;
                
		ajout_deb(&jobs, (void*)&job);
                
                job = (Job*)(jobs->val);
                ajout_deb(&(job->processes), (void*)malloc(sizeof(int)));
		*((int*)((job->processes)->val)) = client_socket;
	    }
	}

	if(errno == EWOULDBLOCK || errno == EAGAIN) //traitement
	{
	    //fprintf(stderr, "No pending connection, processing connected processes");
	    temp = jobs;
	    while(temp)
	    {
                int i = 0;
		temp2 = ((Job*)(temp->val))->processes;
		while(temp2)
		{
		    int red = read(*(int*)(temp2->val), (void*)&instruction, sizeof(long));
                    if(red < 0)
                    {
                        perror("read");
                        exit(1);
                    }
		    if(red)
		    {
			safe_read(*(int*)(temp2->val), sizeof(long) - red, ((void*)&instruction)+red);
		        fprintf(stderr, "%ld\n", instruction);
                        traitement((Job*)(temp->val), *(int*)(temp2->val), i, instruction );
		    }
		    temp2 = temp2->suiv;

                    i++;
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

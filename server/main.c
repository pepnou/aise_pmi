#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
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

//#define MAP_SIZE 4096

typedef struct
{
    long size;
    char* val;
} Message;


typedef struct 
{
    long jobid;
    long barrier;
    long nb_processes;
    Queue processes;
    HashTab hash_tab;
} Job;

void freeMsg(Message* msg)
{
    free(msg->val);
    free(msg);
}

void freeJob(Job* job)
{
    Queue tmp1;
    Queue tmp2;

    tmp1 = job->processes;
    while(tmp1)
    {
        tmp2 = tmp1->suiv;
        
        Comm* comm = (Comm*)tmp1->val;
        
        if(comm->fd == -1)
        {
            close(comm->fd);
        }
        else
        {
            munmap(comm->in, SHM_SIZE);
            munmap(comm->out, SHM_SIZE);
        }

        free((Comm*)(tmp1->val));
        free(tmp1);

        tmp1 = tmp2;
    }

    Data* temp_data;
    Message* temp_msg;

    for(int i = 0; i < HASH_TAB_SIZE; i++)
    {
        tmp1 = job->hash_tab[i];
        while(tmp1)
        {
            tmp2 = tmp1->suiv;

            temp_data = (Data*)(tmp1->val);
            temp_msg = (Message*)(temp_data->val);
            freeMsg(temp_msg);
            freeKey(temp_data->key);
            free(temp_data);
            free(tmp1);

            tmp1 = tmp2;
        }
    }

    free(job);
}

void traitement(Queue* jobs, int job_num, Job* job, int process_num, Comm* comm, long instruction)
{
    Key key;
    init_key(&key);
    long size = 0;
    Message* msg;

    switch(instruction)
    {
        case -2: //barrier
        {
        	job->barrier = job->barrier + 1;

        	if(job->barrier == job->nb_processes)
        	{
        		job->barrier = 0;
        		Queue tmp = job->processes;
        		while(tmp)
        		{
                                long c = 1;
        			safe_write((Comm*)tmp->val, (char*)&c, sizeof(long), 0);
        			tmp = tmp->suiv;
        		}
        	}

        	break;
        }
        case -1: //processus end 
        {
            free(supprElem(&(job->processes), process_num));
            job->nb_processes = job->nb_processes - 1;

            printf("nb_processes : %ld\n", job->nb_processes);

            if(job->nb_processes == 0)
            {
                Job* j = (Job*)supprElem(jobs, job_num);
                freeJob(j);
            }
            break;
        }
        case -3: //get value
        {
            safe_read(comm, (char*)key, KEY_SIZE / 8, 0);
            msg = (Message*)getValue(job->hash_tab, key);

            if(msg)
            {
                //fprintf(stdout, "%s %ld\n", msg->val, msg->size);
                
                safe_write(comm, (char*)&(msg->size), sizeof(long), 0);
                safe_write(comm, (char*)(msg->val), msg->size, 0);
            }
            else
            {
                size = -1;
                safe_write(comm, (char*)&size, sizeof(long), 0);
            }
            break;
        }
        default: //set value of size 'instruction'
        {
            safe_read(comm, (char*)key, KEY_SIZE / 8, 0);
            msg = malloc(sizeof(Message));
            msg->size = instruction;
            msg->val = malloc(msg->size * sizeof(char));
            if(msg->val == NULL)
            {
                perror("malloc");
                exit(1);
            }
            safe_read(comm, msg->val, msg->size, 0);

            //fprintf(stdout, "%s\n", msg->val);

            void* previous_val = setValue(&(job->hash_tab), key, (void*)msg);
            if(previous_val)
                freeMsg((Message*)previous_val);
            break;
        }
    }

    freeKey(key);
}

int create_server(int argc, char** argv)
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

    return listen_sock;
}


int main( int argc, char ** argv )
{
    system("mkdir -p map");
    int listen_sock = create_server(argc, argv);

    /* On va maintenant accepter une connexion */

    /* Ceci sera remplis par
     * accept et décrit le client (ip port)*/
    struct sockaddr client_info;
    socklen_t addr_len = sizeof(client_info);



    Queue jobs = NULL;
    Queue temp = NULL;
    Queue temp2 = NULL;
    long jobid, instruction, nb_processes;
    Job* job;
    //int* temp_fd;
    Comm* comm;
    char comm_type;

    fprintf(stderr, "server online\n");
    int client_socket;

    while(1)
    {
    	//fprintf(stderr, "bonsoir");
    	/* On accepte tous les clients et on récupére leur nouveau FD */
    	while((client_socket = accept(listen_sock, &client_info, &addr_len)) != -1)
    	{
            comm = malloc(sizeof(Comm));

            /* On passe listen_sock en non-blocant */
            int flags = fcntl(client_socket, F_GETFL);
            if(flags == -1)
            {
                perror("get flags");
                exit(1);
            }
            int ret = fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
            if(ret == -1)
            {
                perror("get flags");
                exit(1);
            }


    	    jobid = nb_processes = 0;
    	    safe_read_fd(client_socket, (char*)&jobid, sizeof(long), 0);
    	    safe_read_fd(client_socket, (char*)&nb_processes, sizeof(long), 0);
            safe_read_fd(client_socket, &comm_type, 1, 0);

            if(comm_type == 1)
            {
                comm->fd = client_socket;
            }
            else
            {
                long rank;
                safe_read_fd(client_socket, (char*)&rank, sizeof(long), 0);

                char* file_name = malloc(1024*sizeof(char));
                int mmap_fd = 0;
                                
                sprintf(file_name, "./map/%ld_%ld_0", jobid, rank);
                mmap_fd = open(file_name, O_CREAT | O_RDWR,0666);
                //ftruncate(mmap_fd, SHM_SIZE);
                comm->in = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, 0);
                if(comm->in == MAP_FAILED)
                {
                    perror("mmap");
                    exit(1);
                }
                //memset(comm->in, 0, SHM_SIZE);
                //msync(comm->in, SHM_SIZE, MS_SYNC | MS_INVALIDATE);*/

                sprintf(file_name, "./map/%ld_%ld_1", jobid, rank);
                mmap_fd = open(file_name, O_CREAT | O_RDWR,0666);
                //ftruncate(mmap_fd, SHM_SIZE);
                comm->out = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, 0);
                if(comm->out == MAP_FAILED)
                {
                    perror("mmap");
                    exit(1);
                }
                //memset(comm->out, 0, SHM_SIZE);
                //msync(comm->out, SHM_SIZE, MS_SYNC | MS_INVALIDATE);

                comm->in_offset = 0;
                comm->out_offset = 0;

                close(comm->fd);
                comm->fd = -1;
            }

    	    fprintf(stderr, "new connection for job %ld composed of %ld processes\n", jobid, nb_processes);

    	    temp = jobs;
    	    while(temp)
    	    {
                job = (Job*)(temp->val);
                
                if(job->jobid == jobid)
                {
                    /*temp_fd = malloc(sizeof(int));
                    *temp_fd = client_socket;*/
                    ajout_deb(&(job->processes), (void*)comm);
    	    
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
                    job->jobid = jobid;
                    init_hashtab(&(job->hash_tab));
                    
                    ajout_deb(&jobs, (void*)job);

                    /*temp_fd = malloc(sizeof(int));
                    *temp_fd = client_socket;*/
                    ajout_deb(&(job->processes), (void*)comm);
    	    }
    	}

    	if(errno == EWOULDBLOCK || errno == EAGAIN) //traitement
    	{
    	    //fprintf(stderr, "No pending connection, processing connected processes");
    	    temp = jobs;
            int job_num = 0;
    	    while(temp)
    	    {
                int process_num = 0;
        	temp2 = ((Job*)(temp->val))->processes;
        	while(temp2)
        	{ 
        	    int red = comm_read(*(Comm*)(temp2->val), (void*)&instruction, sizeof(long));

        	    if(red)
        	    {
                        if(((Comm*)temp2->val)->fd == -1)
                            safe_read((Comm*)temp2->val, (char*)&instruction, sizeof(long), 0);
                        else
                            safe_read((Comm*)temp2->val, (char*)&instruction, sizeof(long) - red, red);
                        //fprintf(stdout, "%ld\n", instruction);
                        //if(instruction > 10)
                        //    exit(2);
                        traitement(&jobs, job_num, (Job*)(temp->val), process_num, (Comm*)temp2->val, instruction );
                    }
                    if(instruction != -1)
                    {
                        temp2 = temp2->suiv;
                        process_num++;
                    }
                    
                    else
                        break;
                }
                if(instruction != -1)
                {
                    temp = temp->suiv;
                    job_num++;
                }
                else
                    break;
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

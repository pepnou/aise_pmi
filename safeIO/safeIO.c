#include "safeIO.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <math.h>


int comm_read(Comm comm, char* buf, int size)
{
    if(comm.fd == -1)
    {
        int offset;

        if(comm.in_offset + size + 1 > SHM_SIZE)
            offset = 0;
        else
            offset = comm.in_offset;

        char* cmp =  malloc(size);
        memset(cmp, 0, size);
        
        if(!memcmp(cmp, &(comm.in[offset + 1]), size))
        {
            free(cmp);
            return 0;
        }
        else
        {
            free(cmp);
            return 1;
        }
    }
    else
    {
        int red = read(comm.fd, buf, size);
        if(red == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                red = 0;
            else
            {
                perror("read");
                exit(1);
            }
        }
        return red;
    }
}

void safe_read(Comm* comm, char* buf, int size, int offset)
{
    if(comm->fd == -1)
    {
        if(comm->in_offset + size + 1 > SHM_SIZE)
            offset = 0;
        else
            offset = comm->in_offset;

        safe_read_shm(comm->in, buf, size, offset);

        comm->in_offset = (comm->in_offset + size + 1) % (SHM_SIZE - 1);
    }
    else
    {
        safe_read_fd(comm->fd, buf, size, offset);
    }
}

void safe_write(Comm* comm, char* buf, int size, int offset)
{
    if(comm->fd == -1)
    {
        if(comm->out_offset + size + 1 > SHM_SIZE)
        {
            char* cmp = malloc(SHM_SIZE - comm->out_offset);
            memset(cmp, 0, SHM_SIZE - comm->out_offset);
            while(memcmp(&(comm->out[comm->out_offset]), cmp, SHM_SIZE - comm->out_offset));
            free(cmp);

            offset = 0;
        }
        else
            offset = comm->out_offset;

        safe_write_shm(comm->out, buf, size, offset);

        comm->out_offset = (comm->out_offset + size + 1) % (SHM_SIZE - 1);
    }
    else
    {
        safe_write_fd(comm->fd, buf, size, offset);
    }
}

void safe_read_fd(int fd, char* buf, int size, int offset)
{
    int red = read(fd, &(buf[offset]), size);
    
    if(red == -1)
    {
        if(errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("safe_read");
            exit(1);
        }
        else
        {
            usleep(10);
            safe_read_fd(fd, buf, size, offset);
        }
    }
    else if(size - red)
        safe_read_fd(fd, buf, size - red, offset + red);
}

void safe_write_fd(int fd, char* buf, int size, int offset)
{
    int wrote = write(fd, &(buf[offset]), size);

    if(wrote == -1)
    {
        if(errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("safe_write");
            exit(1);
        }
        else
            safe_write_fd(fd, buf, size, offset);
    }
    else if(size - wrote)
        safe_write_fd(fd, buf, size - wrote, offset + wrote);
}

void safe_read_shm(char* in, char* buf, int size, int offset)
{
    char* cmp =  malloc(size);
    memset(cmp, 0, size);

    while(!memcmp(cmp, &(in[offset + 1]), size));
    
    free(cmp);

    int deb = (offset/PAGE_SIZE) * PAGE_SIZE;
    int nb_page = ceil((double)(offset - deb + (size + 1))/PAGE_SIZE);

    int ret = mlock(&(in[deb]), nb_page * PAGE_SIZE);
    if(ret == -1)
    {
        perror("mlock");
        exit(1);
    }
 
    memcpy(buf, &(in[offset + 1]), size);
    memset(&(in[offset]), 0, size + 1);
    
    ret = munlock(&(in[deb]), nb_page * PAGE_SIZE);
    if(ret == -1)
    {
        perror("munlock");
        exit(1);
    }

    //int ret = msync(&(in[offset]), size + 1, MS_ASYNC | MS_INVALIDATE);
    ret = msync(&(in[deb]), nb_page * PAGE_SIZE, MS_ASYNC | MS_INVALIDATE);
    if(ret == -1)
    {
        fprintf(stderr, "%d %d %d %d\n", errno, EBUSY, EINVAL, ENOMEM);
        fprintf(stderr, "%d %d %d\n", deb, nb_page, nb_page*PAGE_SIZE);
        perror("msync");
        exit(1);
    }
}

void safe_write_shm(char* out, char* buf, int size, int offset)
{
    char* cmp =  malloc(size + 1);
    memset(cmp, 0, size + 1);

    while(memcmp(cmp, &(out[offset]), size + 1));

    free(cmp);

    int deb = (offset/PAGE_SIZE) * PAGE_SIZE;
    int nb_page = ceil((double)(offset - deb + (size + 1))/PAGE_SIZE);

    int ret = mlock(&(out[deb]), nb_page * PAGE_SIZE);
    if(ret == -1)
    {
        perror("mlock");
        exit(1);
    }

    memset(&(out[offset]), 0xFF, 1);
    memcpy(&(out[offset + 1]), buf, size);

    ret = munlock(&(out[deb]), nb_page * PAGE_SIZE);
    if(ret == -1)
    {
        perror("munlock");
        exit(1);
    }

    //int ret = msync(&(out[offset]), size + 1, MS_ASYNC | MS_INVALIDATE);
    ret = msync(&(out[deb]), nb_page * PAGE_SIZE, MS_ASYNC | MS_INVALIDATE);
    if(ret == -1)
    {
        fprintf(stderr, "%d %d %d %d\n", errno, EBUSY, EINVAL, ENOMEM);
        fprintf(stderr, "%d %d %d\n", deb, nb_page, nb_page*PAGE_SIZE);
        perror("msync");
        exit(1);
    }
}





/*void flush_shm(Comm comm)
{
    if(comm.fd == -1)
    {

    }
}*/


void safe_read_non_block(Comm* comm, char* buf, int size, int offset)
{
    if(comm->fd == -1)
    {
        if(comm->in_offset + size + 1 > SHM_SIZE)
            offset = 0;
        else
            offset = comm->in_offset;

        safe_read_shm_non_block(comm->in, buf, size, offset);

        comm->in_offset = (comm->in_offset + size + 1) % (SHM_SIZE - 1);
    }
    else
    {
        safe_read_fd(comm->fd, buf, size, offset);
    }
}

void safe_write_non_block(Comm* comm, char* buf, int size, int offset)
{
    if(comm->fd == -1)
    {
        if(comm->out_offset + size + 1 > SHM_SIZE)
        {
            char* cmp = malloc(SHM_SIZE - comm->out_offset);
            memset(cmp, 0, SHM_SIZE - comm->out_offset);
            while(memcmp(&(comm->out[comm->out_offset]), cmp, SHM_SIZE - comm->out_offset));
            free(cmp);

            offset = 0;
        }
        else
            offset = comm->out_offset;

        safe_write_shm_non_block(comm->out, buf, size, offset);

        comm->out_offset = (comm->out_offset + size + 1) % (SHM_SIZE - 1);
    }
    else
    {
        safe_write_fd(comm->fd, buf, size, offset);
    }
}

void safe_read_shm_non_block(char* in, char* buf, int size, int offset)
{
    char* cmp =  malloc(size);
    memset(cmp, 0, size);

    while(!memcmp(cmp, &(in[offset + 1]), size));
    
    free(cmp);

    int deb = (offset/PAGE_SIZE) * PAGE_SIZE;
    int nb_page = ceil((double)(offset - deb + (size + 1))/PAGE_SIZE);

    /*int ret = mlock(&(in[deb]), nb_page * PAGE_SIZE);
    if(ret == -1)
    {
        perror("mlock");
        exit(1);
    }*/
 
    memcpy(buf, &(in[offset + 1]), size);
    memset(&(in[offset]), 0, size + 1);
    
    /*ret = munlock(&(in[deb]), nb_page * PAGE_SIZE);
    if(ret == -1)
    {
        perror("munlock");
        exit(1);
    }

    //int ret = msync(&(in[offset]), size + 1, MS_ASYNC | MS_INVALIDATE);
    ret = msync(&(in[deb]), nb_page * PAGE_SIZE, MS_ASYNC | MS_INVALIDATE);
    if(ret == -1)
    {
        fprintf(stderr, "%d %d %d %d\n", errno, EBUSY, EINVAL, ENOMEM);
        fprintf(stderr, "%d %d %d\n", deb, nb_page, nb_page*PAGE_SIZE);
        perror("msync");
        exit(1);
    }*/
}

void safe_write_shm_non_block(char* out, char* buf, int size, int offset)
{
    char* cmp =  malloc(size + 1);
    memset(cmp, 0, size + 1);

    while(memcmp(cmp, &(out[offset]), size + 1));

    free(cmp);

    int deb = (offset/PAGE_SIZE) * PAGE_SIZE;
    int nb_page = ceil((double)(offset - deb + (size + 1))/PAGE_SIZE);

    /*int ret = mlock(&(out[deb]), nb_page * PAGE_SIZE);
    if(ret == -1)
    {
        perror("mlock");
        exit(1);
    }*/

    memset(&(out[offset]), 0xFF, 1);
    memcpy(&(out[offset + 1]), buf, size);

    /*ret = munlock(&(out[deb]), nb_page * PAGE_SIZE);
    if(ret == -1)
    {
        perror("munlock");
        exit(1);
    }

    //int ret = msync(&(out[offset]), size + 1, MS_ASYNC | MS_INVALIDATE);
    ret = msync(&(out[deb]), nb_page * PAGE_SIZE, MS_ASYNC | MS_INVALIDATE);
    if(ret == -1)
    {
        fprintf(stderr, "%d %d %d %d\n", errno, EBUSY, EINVAL, ENOMEM);
        fprintf(stderr, "%d %d %d\n", deb, nb_page, nb_page*PAGE_SIZE);
        perror("msync");
        exit(1);
    }*/
}
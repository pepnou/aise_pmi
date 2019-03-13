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

    //fprintf(stderr, "%d\n", offset);
 
    memcpy(buf, &(in[offset + 1]), size);
    //for(int i = 0; i < size; i++)
    //    buf[i] = in[offset + 1 + i];
    
    memset(&(in[offset]), 0, size + 1);
    
    int deb = (offset/PAGE_SIZE) * PAGE_SIZE;
    int nb_page = ceil((double)(offset - deb + (size + 1))/PAGE_SIZE);

    //int ret = msync(&(out[offset]), size + 1, MS_SYNC | MS_INVALIDATE);
    int ret = msync(&(in[deb]), nb_page * PAGE_SIZE, MS_ASYNC | MS_INVALIDATE);
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
    //fprintf(stderr, "%s\n", buf);

    char* cmp =  malloc(size + 1);
    memset(cmp, 0, size + 1);

    while(memcmp(cmp, &(out[offset]), size + 1));

    free(cmp);

    //fprintf(stderr, "%d\n", offset);
    
    memset(&(out[offset]), 0xFF, 1);
    memcpy(&(out[offset + 1]), buf, size);
    //for(int i = 0; i < size; i++)
    //    out[offset + 1 + i] = buf[i];

    //if(size > 2 && size < 7)
    //    fprintf(stdout, "write : %s\n", &(out[offset + 1]));
    
    int deb = (offset/PAGE_SIZE) * PAGE_SIZE;
    int nb_page = ceil((double)(offset - deb + (size + 1))/PAGE_SIZE);

    //int ret = msync(&(out[offset]), size + 1, MS_SYNC | MS_INVALIDATE);
    int ret = msync(&(out[deb]), nb_page * PAGE_SIZE, MS_ASYNC | MS_INVALIDATE);
    if(ret == -1)
    {
        fprintf(stderr, "%d %d %d %d\n", errno, EBUSY, EINVAL, ENOMEM);
        fprintf(stderr, "%d %d %d\n", deb, nb_page, nb_page*PAGE_SIZE);
        perror("msync");
        exit(1);
    }
}

/*void flush_shm(char* shm, int deb, int fin)
{
    if(fin < deb)
    {
        flush_shm(shm, deb, SHM_SIZE - 1);
        flush_shm(shm, 0, fin);
    }
    else
    {
        
    }
}*/

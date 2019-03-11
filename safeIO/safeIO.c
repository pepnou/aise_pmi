#include "safeIO.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>


int comm_read(Comm comm, char* buf, int size)
{
    if(comm.fd == -1)
    {
        int offset;
        if(comm.in_offset + size >= SHM_SIZE)
            offset = 0;
        else
            offset = comm.in_offset;

        char* cmp =  malloc(size);
        memset(cmp, 0, size);
        
        if(!memcmp(cmp, &(comm.in[offset]), size))
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
        if(comm->in_offset + size >= SHM_SIZE)
            offset = 0;
        else
            offset = comm->in_offset;

        safe_read_shm(comm->in, buf, size, offset);

        comm->in_offset = (comm->in_offset + size) % SHM_SIZE;
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
        if(comm->out_offset + size >= SHM_SIZE)
            offset = 0;
        else
            offset = comm->out_offset;

        safe_write_shm(comm->out, buf, size, offset);

        comm->out_offset = (comm->out_offset + size) % SHM_SIZE;
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
            safe_read_fd(fd, buf, size, offset);
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

    while(!memcmp(cmp, &(in[offset]), size));
    
    free(cmp);

    memcpy(buf, &(in[offset]), size);
    memset(&(in[offset]), 0, size);
    msync(&(in[offset]), size, MS_SYNC | MS_INVALIDATE);
}

void safe_write_shm(char* out, char* buf, int size, int offset)
{
    char* cmp =  malloc(size);
    memset(cmp, 0, size);

    while(memcmp(cmp, &(out[offset]), size));

    free(cmp);

    memcpy(&(out[offset]), buf, size);
    msync(&(out[offset]), size, MS_SYNC | MS_INVALIDATE);
}

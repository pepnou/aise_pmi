#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>


void safe_read(int fd, char* buf, int size, int offset)
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
            safe_read(fd, buf, size, offset);
    }
    else if(size - red)
        safe_read(fd, buf, size - red, offset + red);
}

void safe_write(int fd, char* buf, int size, int offset)
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
            safe_write(fd, buf, size, offset);
    }
    else if(size - wrote)
        safe_write(fd, buf, size - wrote, offset + wrote);
}

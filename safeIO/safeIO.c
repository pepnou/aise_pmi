#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


void safe_read(int fd, char* buf, int size, int offset)
{
	/*char* b = (char*)buf;

	int red = 0;
	while(red != size)
	{
		red = read(fd, &(b[red]), size - red);
		if(red == -1)
		{
			perror("safe_read");
			exit(1);
		}
	}*/

    int red = read(fd, &(buf[offset]), size);
    
    if(red == -1)
    {
        perror("safe_read");
        exit(1);
    }
    else if(size - red)
        safe_read(fd, buf, size - red, offset + red);
}

void safe_write(int fd, char* buf, int size, int offset)
{
	/*char* b = (char*)buf;

	int wrote = 0;
	while(wrote != size)
	{
		wrote = write(fd, &(b[wrote]), size - wrote);
		if(wrote == -1)
		{
			perror("safe_write");
			exit(1);
		}
	}*/
    int wrote = write(fd, &(buf[offset]), size);

    if(wrote == -1)
    {
        perror("safe_write");
        exit(1);
    }
    else if(size - wrote)
        safe_write(fd, buf, size - wrote, offset + wrote);
}

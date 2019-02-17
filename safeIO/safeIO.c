


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

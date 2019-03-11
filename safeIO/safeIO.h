#ifndef __SAFEIO__
#define __SAFEIO__

#define SHM_SIZE 4096

typedef struct
{
    int fd;
    char* in;
    char* out;
} Comm;

int comm_read(Comm comm, char* buf, int size);

void safe_read(Comm comm, char* buf, int size, int offset);
void safe_write(Comm comm, char* buf, int size, int offset);

void safe_read_fd(int fd, char* buf, int size, int offset);
void safe_write_fd(int fd, char* buf, int size, int offset); 

void safe_read_shm(char* out, char* buf, int size);
void safe_write_shm(char* in, char* buf, int size); 


#endif

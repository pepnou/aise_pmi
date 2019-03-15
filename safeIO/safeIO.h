#ifndef __SAFEIO__
#define __SAFEIO__

//#define SHM_SIZE 4096
#define SHM_SIZE 16777216
#define PAGE_SIZE 4096

typedef struct
{
    int fd;
    
    char* in;
    char* out;
    
    int in_offset;
    int out_offset;
} Comm;

int comm_read(Comm comm, char* buf, int size);

void safe_read(Comm* comm, char* buf, int size, int offset);
void safe_write(Comm* comm, char* buf, int size, int offset);

void safe_read_fd(int fd, char* buf, int size, int offset);
void safe_write_fd(int fd, char* buf, int size, int offset); 

void safe_read_shm(char* out, char* buf, int size, int offset);
void safe_write_shm(char* in, char* buf, int size, int offset);

//void flush_shm(char* shm, int size, int offset);

void safe_read_non_block(Comm* comm, char* buf, int size, int offset);
void safe_write_non_block(Comm* comm, char* buf, int size, int offset);

void safe_read_shm_non_block(char* in, char* buf, int size, int offset);
void safe_write_shm_non_block(char* out, char* buf, int size, int offset);

#endif

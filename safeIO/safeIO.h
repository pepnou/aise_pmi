#ifndef __SAFEIO__
#define __SAFEIO__


void safe_read(int fd, int size, void* buf);
void safe_write(int fd, int size, void* buf);


#endif

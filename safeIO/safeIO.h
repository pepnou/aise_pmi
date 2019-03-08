#ifndef __SAFEIO__
#define __SAFEIO__


void safe_read(int fd, char* buf, int size, int offset);
void safe_write(int fd, char* buf, int size, int offset);


#endif

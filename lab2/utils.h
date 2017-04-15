#ifndef __UTILS__H__
#define __UTILS__H__

#include <sys/stat.h>

#define CONVERT(type, value) (*(type*)(value))
#define VALID(type, value) ((value) && (CONVERT(type, value)))

char *mygetenv(const char*);
int readlink_fd(int, char*, int);
const char *filetype(struct stat);
const char *fileperm(struct stat);
void print(const char *, const void *);


#endif

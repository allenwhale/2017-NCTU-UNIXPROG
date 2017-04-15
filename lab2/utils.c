#include "utils.h"
#include "inject.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <stdlib.h>

extern char **environ;

char *mygetenv(const char *name) {
    if(name == NULL || environ == NULL)
        return NULL;
    int len = strlen(name);
    for(char **p = environ ; *p ; ++p) {
        if(!strncmp(name, *p, len) && (*p)[len] == '=')
            return *p + len + 1;
    }
    return NULL;
}

int readlink_fd(int fd, char *filepath, int sz) {
    static ssize_t (*old_readlink)(const char*, char*, size_t) = NULL;
    static int (*old_getpid)() = NULL;
    if(!old_readlink) old_readlink = (ssize_t(*)(const char*, char*, size_t))load_sym("readlink");
    if(!old_getpid) old_getpid = (int(*)())load_sym("getpid");
    memset(filepath, 0, sizeof(char) * sz);
    char procpath[PATH_MAX];
    sprintf(procpath, "/proc/%d/fd/%d", old_getpid(), fd);
    return old_readlink(procpath, filepath, sz);
}

static const char *_filetype[] = {
    "unknown",
    "block device",
    "character device",
    "directory",
    "FIFO/pipe",
    "symlink",
    "regular file",
    "socket",
    NULL
};

const char *filetype(struct stat st) {
    int sel = 0;
    if(S_ISBLK(st.st_mode)) sel = 1;
    else if(S_ISCHR(st.st_mode)) sel = 2;
    else if(S_ISDIR(st.st_mode)) sel = 3;
    else if(S_ISFIFO(st.st_mode)) sel = 4;
    else if(S_ISLNK(st.st_mode)) sel = 5;
    else if(S_ISREG(st.st_mode)) sel = 6;
    /*else if(S_ISSOCK(st.st_mode)) sel = 7;*/
    return _filetype[sel];
}


const char *fileperm(struct stat st) {
    static char _fileperm[16];
    _fileperm[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    _fileperm[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    _fileperm[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    _fileperm[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    _fileperm[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    _fileperm[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    _fileperm[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
    _fileperm[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    _fileperm[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    _fileperm[9] = 0;
    return _fileperm;
}

void print(const char *_type, const void *value) {
    char type[16] = {0};
    int i = 0;
    for(const char *p = _type ; *p ; ++p)
        if(*p != ' ' && *p != '(' && *p != ')')type[i++] = *p;
    if(!strcmp(type, "char")) {
        mprintf("\'%c\'", *((char*)value));
    } else if(!strcmp(type, "int")){
        mprintf("%d", *((int*)value));
    } else if(!strcmp(type, "unsignedint") || !strcmp(type, "unsigned")){
        mprintf("%u", *((unsigned int*)value));
    } else if(!strcmp(type, "size_t")){
        mprintf("%lu", *((size_t*)value));
    } else if(!strcmp(type, "ssize_t")){
        mprintf("%d", *((ssize_t*)value));
    } else if(!strcmp(type, "short")){
        mprintf("%i", *((short*)value));
    } else if(!strcmp(type, "long") || !strcmp(type, "off_t")){
        mprintf("%ld", *((long*)value));
    } else if(!strcmp(type, "long long")){
        mprintf("%lld", *((long long*)value));
    } else if(!strcmp(type, "float")){
        mprintf("%f", *((float*)value));
    } else if(!strcmp(type, "double")){
        mprintf("%f", *((double*)value));
    } else if(!strcmp(type, "mode_t")){
        mprintf("0%03o", *((mode_t*)value));
    } else if(!strcmp(type, "char*")){
        if(VALID(char *, value)) {
            const char *ptr = CONVERT(const char*, value);
            PRINT_ARG(const char *, ptr);
        } else {
            mprintf("%p", CONVERT(void*, value));
        }
    } else if(!strcmp(type, "constchar*")){
        if(VALID(const char*, value)){
            int i;
            const char *ptr = CONVERT(const char *, value);
            mprintf("\"");
            for(i = 0 ; ptr[i] && i < 50 ; i++) {
                if(isprint(ptr[i]))
                    mprintf("%c", ptr[i]);
                else if(ptr[i] == '\n')
                    mprintf("\\n");
                else if(ptr[i] == '\r')
                    mprintf("\\r");
                else if(ptr[i] == '\t')
                    mprintf("\\t");
                else
                    mprintf("\\x%02X", ptr[i]);
            }
            if(ptr[i]) mprintf("...");
            mprintf("\"");
        } else {
            mprintf("%p", CONVERT(void*, value));
        }
    } else if(!strcmp(type, "char**")){
        if(VALID(char **, value)) {
            const char **ptr = CONVERT(const char**, value);
            PRINT_ARG(const char**, ptr);
        } else {
            mprintf("%p", CONVERT(void*, value));
        }
    } else if(!strcmp(type, "constchar**")){
        if(VALID(const char**, value)){
            int i = 0;
            const char **p;
            mprintf("[");
            for(p = CONVERT(const char**, value) ; *p && i < 5 ; ++p, ++i){
                if(i) mprintf(", ");
                PRINT_ARG(const char*, *p);
            }
            if(*p) mprintf(", ...");
            mprintf("]");
        } else {
            mprintf("%p", CONVERT(void*, value));
        }
    } else if(!strcmp(type, "fd_t")){
        char *filepath = (char*)malloc(sizeof(char) * PATH_MAX);
        if(readlink_fd(CONVERT(int, value), filepath, PATH_MAX) != -1){
            mprintf("<fd=%d, path=\"%s\">", CONVERT(int, value), filepath);
        }else{
            mprintf("%d", CONVERT(int, value));
        }
    } else if(!strcmp(type, "FILE*")){
        if(VALID(FILE*, value)){
            int fd = fileno(CONVERT(FILE*, value));
            PRINT_ARG(fd_t, fd);
        }else{
            mprintf("%p", CONVERT(void*, value));
        }
    } else if(!strcmp(type, "DIR*")){
        if(VALID(DIR*, value)){
            int fd = dirfd(CONVERT(DIR*, value));
            PRINT_ARG(fd_t, fd);
        }else{
            mprintf("%p", CONVERT(void*, value));
        }
    } else if(!strcmp(type, "uid_t")){
        struct passwd *pw = getpwuid(CONVERT(int, value));
        if(pw){
            mprintf("<user=%s>", pw->pw_name);
        }else{
            mprintf("(unknown)");
        }
    } else if(!strcmp(type, "gid_t")){
        struct group *gr = getgrgid(CONVERT(int, value));
        if(gr){
            mprintf("<group=%s>", gr->gr_name);
        }else{
            mprintf("(unknown)");
        }
    } else if(!strcmp(type, "structstat")){
        struct stat st = CONVERT(struct stat, value);
        mprintf("<size=%d, type=%s, perm=%s>", st.st_size, filetype(st), fileperm(st));
    } else if(!strcmp(type, "structstat*")){
        if(VALID(struct stat*, value)){
            struct stat st = *CONVERT(struct stat*, value);
            PRINT_ARG(struct stat, st);
        }else{
            mprintf("%p", CONVERT(void*, value));
        }
    } else if (!strcmp(type, "structdirent*")){
        if(VALID(struct dirent*, value)){
            const char *ptr = CONVERT(struct dirent*, value)->d_name;
            PRINT_ARG(const char*, ptr);
        }else{
            mprintf("%p", CONVERT(void*, value));
        }
    } else if (!strcmp(type, "structdirent**")){
        if(VALID(struct dirent**, value) && *(CONVERT(struct dirent**, value))){
            struct dirent *ptr = *CONVERT(struct dirent**, value);
            PRINT_ARG(struct dirent*, ptr);
        }else{
            mprintf("%p", CONVERT(void*, value));
        }
    } else if (!strcmp(type, "constvoid*")){
        if(VALID(const void*, value)){
            const char *ptr = CONVERT(const char*, value);
            PRINT_ARG(const char*, ptr);
        } else {
            mprintf("%p", CONVERT(void*, value));
        }
    } else if(!strcmp(type, "void*")) {
        if(VALID(void*, value)) {
            const char *ptr = CONVERT(const char*, value);
            PRINT_ARG(const char*, ptr);
        } else {
            mprintf("%p", CONVERT(void*, value));
        }
    } else {
        mprintf("%p", value);
    }
}

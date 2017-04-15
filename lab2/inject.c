#include "inject.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <unistd.h>
#include <dlfcn.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>

FILE *get_output_fd() {
    static char output_path[PATH_MAX];
    static int first = 1;
    const char *env_output_path = mygetenv("MONITOR_OUTPUT");
    static char (*old_getcwd)(char*, size_t) = NULL;
    if(env_output_path && strcmp(env_output_path, "stderr")) {
        if(first) {
            if(env_output_path[0] == '/') {
                strcpy(output_path, env_output_path);
            } else {
                if(!old_getcwd)old_getcwd = (char(*)(char*, size_t))load_sym("getcwd"); 
                old_getcwd(output_path, PATH_MAX);
                strcat(output_path, "/");
                strcat(output_path, env_output_path);
            }
        }
        FILE *res = fopen(output_path, "a");
        first = 0;
        return res;
    } else {
        return stderr;
    }
}

void close_output_fd(FILE *fd) {
    if(fd != stderr) fclose(fd);
}

void mprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    FILE *output_fd = get_output_fd();
    vfprintf(output_fd, fmt, ap);
    va_end(ap);
    fflush(output_fd);
    close_output_fd(output_fd);
}

void *load_sym(const char *sym) {
    void *handler = dlopen("libc.so.6", RTLD_LAZY);
    void *res = dlsym(handler, sym);
    dlclose(handler);
    return res;
} 

fd_t open(const char *pathname, int flags, ...) {
    static int (*old_open)(const char*, int, ...) = NULL;
    if(!old_open) old_open = (int(*)(const char*, int, ...))load_sym("open");
    mprintf("[monitor] open(");
    PRINT_ARG(const char*, pathname);
    mprintf(", ");
    PRINT_ARG(int, flags);
    int res = 0;
    if(flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode_t mode = va_arg(ap, mode_t);
        va_end(ap);
        mprintf(", ");
        PRINT_ARG(mode_t, mode);
        res = old_open(pathname, flags, mode);
    } else {
        res = old_open(pathname, flags);
    }
    mprintf(") = ");
    PRINT_ARG(fd_t, res);
    mprintf("\n");
    return res;
}

int execl(const char *path, const char *arg, ...) {
    static int (*old_execve)(const char*, char* const*, char* const*) = NULL;
    if(!old_execve) old_execve = (int(*)(const char*, char* const*, char* const*))load_sym("execve");
    mprintf("[monitor] execl(");
    PRINT_ARG(const char*, path);
    mprintf(", ");
    PRINT_ARG(const char*, arg);
    extern char **environ;
    int argc = 0;
    va_list ap;
    va_start(ap, arg);
    for(char *p = (char*)arg ; p ; p = va_arg(ap, char*), argc++) {
        if(argc == INT_MAX) {
            va_end(ap);
            errno = E2BIG;
            return -1;
        }
    }
    char *argv[argc + 1];
    va_start(ap, arg);
    argc = 0;
    for(char *p = (char*)arg ; p ; p = va_arg(ap, char*)) {
        argv[argc] = p;
        mprintf(", ");
        PRINT_ARG(char*, argv[argc++]);
    }
    mprintf(")");
    argv[argc] = 0;
    va_end(ap);
    int res = old_execve(path, argv, environ);
    mprintf(" = ");
    PRINT_ARG(int, res);
    mprintf("\n");
    return res;
}

int execle(const char *path, const char *arg, ...) {
    static int (*old_execve)(const char*, char* const*, char* const*) = NULL;
    if(!old_execve) old_execve = (int(*)(const char*, char* const*, char* const*))load_sym("execve");
    mprintf("[monitor] execle(");
    PRINT_ARG(const char*, path);
    mprintf(", ");
    PRINT_ARG(const char*, arg);
    int argc = 0;
    va_list ap;
    va_start(ap, arg);
    for(char *p = (char*)arg ; p ; p = va_arg(ap, char*), argc++) {
        if(argc == INT_MAX) {
            va_end(ap);
            errno = E2BIG;
            return -1;
        }
    }
    char *argv[argc + 1];
    va_start(ap, arg);
    argc = 0;
    for(char *p = (char*)arg ; p ; p = va_arg(ap, char*)) {
        argv[argc] = p;
        mprintf(", ");
        PRINT_ARG(char*, argv[argc++]);
    }
    char **envp = va_arg(ap, char**);
    mprintf(", ");
    PRINT_ARG(char**, envp);
    mprintf(")");
    argv[argc] = 0;
    va_end(ap);
    int res = old_execve(path, argv, envp);
    mprintf(" = ");
    PRINT_ARG(int, res);
    return res;
}

int execlp(const char *file, const char *arg, ...) {
    mprintf("[monitor] execlp(");
    PRINT_ARG(const char*, file);
    mprintf(", ");
    PRINT_ARG(const char*, arg);
    extern char **environ;
    int argc = 0;
    va_list ap;
    va_start(ap, arg);
    for(char *p = (char*)arg ; p ; p = va_arg(ap, char*), argc++) {
        if(argc == INT_MAX) {
            va_end(ap);
            errno = E2BIG;
            return -1;
        }
    }
    char *argv[argc + 1];
    va_start(ap, arg);
    argc = 0;
    for(char *p = (char*)arg ; p ; p = va_arg(ap, char*)) {
        argv[argc] = p;
        mprintf(", ");
        PRINT_ARG(char*, argv[argc++]);
    }
    mprintf(")");
    argv[argc] = 0;
    va_end(ap);
    int res = execvpe(file, argv, environ);
    mprintf(" = ");
    PRINT_ARG(int, res);
    mprintf("\n");
    return res;
}

int fork() {
    static int (*old_fork)() = NULL;
    if(!old_fork) old_fork = (int(*)())load_sym("fork");
    int ret = old_fork();
    mprintf("[monitor] fork() = %d\n", ret);
    return ret;
}
//int closedir(DIR *dirp);
inject(1, int, closedir, 1, DIR*);
//DIR *fdopendir(int fd);
inject(1, DIR*, fdopendir, 1, fd_t);
//DIR *opendir(const char *name);
inject(1, DIR*, opendir, 1, const char*);
//struct dirent *readdir(DIR *dirp);
inject(1, struct dirent*, readdir, 1, DIR*);
//int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
inject(0, int, readdir_r, 3, DIR*, struct dirent*, struct dirent**);
//void rewinddir(DIR *dirp);
inject_no_ret(1, rewinddir, 1, DIR*);
//void seekdir(DIR *dirp, long loc);
inject_no_ret(1, seekdir, 2, DIR*, long);
//long telldir(DIR *dirp);
inject(1, long, telldir, 1, DIR*);
//int creat(const char *pathname, mode_t mode);
inject(1, int, creat, 2, const char*, mode_t);
//int remove(const char *pathname);
inject(1, int, remove, 1, const char*);
//int rename(const char *oldpath, const char *newpath);
inject(1, int, rename, 2, const char*, const char*);
//void setbuf(FILE *stream, char *buf);
inject_no_ret(1, setbuf, 2, FILE*, char*);
//int setvbuf(FILE *stream, char *buf, int mode, size_t size);
inject(1, int, setvbuf, 4, FILE*, char*, int, size_t);
//char *tempnam(const char *dir, const char *pfx);
inject(1, char*, tempnam, 2, const char*, const char*);
//FILE *tmpfile(void);
inject(1, FILE*, tmpfile, 0);
//char *tmpnam(char *s);
inject(1, char*, tmpnam, 1, char*);
//void exit(int status);
inject_no_ret(1, exit, 1, int);
//char *getenv(const char *name);
inject(1, char*, getenv, 1, const char*);
//char *mkdtemp(char *template);
inject(1, char*, mkdtemp, 1, char*);
//int mkstemp(char *template);
inject(1, int, mkstemp, 1, char*);
//int putenv(char *string);
inject(1, int, putenv, 1, char*);
//int rand(void);
inject(1, int, rand, 0);
//int rand_r(unsigned int *seedp);
inject(1, int, rand_r, 1, unsigned int*);
//int setenv(const char *name, const char *value, int overwrite);
inject(1, int, setenv, 3, const char*, const char*, int);
//void srand(unsigned int seed);
inject_no_ret(1, srand, 1, unsigned int);
//int chdir(const char *path);
inject(1, int, chdir, 1, const char*);
//int chown(const char *pathname, uid_t owner, gid_t group);
inject(1, int, chown, 3, const char*, uid_t, gid_t);
//int close(int fd);
inject(1, int, close, 1, fd_t);
//int dup(int oldfd);
inject(1, fd_t, dup, 1, fd_t);
//int dup2(int oldfd, int newfd);
inject(1, fd_t, dup2, 2, fd_t, fd_t);
//void _exit(int status);
inject_no_ret(1, _exit, 1, int);
//int execv(const char *path, char *const argv[]);
inject(1, int, execv, 2, const char*, char* const*);
//int execve(const char *filename, char *const argv[], char *const envp[]);
inject(1, int, execve, 3, const char*, char* const*, char* const*);
//int execvp(const char *file, char *const argv[]);
inject(1, int, execvp, 2, const char*, char* const*);
//int fchdir(int fd);
inject(1, int, fchdir, 1, fd_t);
//int fchown(int fd, uid_t owner, gid_t group);
inject(1, int, fchown, 3, fd_t, uid_t, gid_t);
//int fsync(int fd);
inject(1, int, fsync, 1, fd_t);
//int ftruncate(int fd, off_t length);
inject(1, int, ftruncate, 2, fd_t, off_t);
//char *getcwd(char *buf, size_t size);
inject(1, char*, getcwd, 2, char*, size_t);
//gid_t getegid(void);
inject(1, gid_t, getegid, 0);
//uid_t geteuid(void);
inject(1, uid_t, geteuid, 0);
//gid_t getgid(void);
inject(1, gid_t, getgid, 0);
//uid_t getuid(void);
inject(1, uid_t, getuid, 0);
//int link(const char *oldpath, const char *newpath);
inject(1, int, link, 2, const char*, const char*);
//int pipe(int pipefd[2]);
inject(1, int, pipe, 1, int*);
//ssize_t pread(int fd, void *buf, size_t count, off_t offset);
inject(0, ssize_t, pread, 4, fd_t, void*, size_t, off_t);
//ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
inject(1, ssize_t, pwrite, 4, fd_t, const void*, size_t, off_t);
//ssize_t read(int fd, void *buf, size_t count);
inject(0, ssize_t, read, 3, fd_t, void*, size_t);
//ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
inject(0, ssize_t, readlink, 3, const char*, char*, size_t);
//int rmdir(const char *pathname);
inject(1, int, rmdir, 1, const char*);
//int seteuid(uid_t euid);
inject(1, int, seteuid, 1, uid_t);
//int setegid(gid_t egid);
inject(1, int, setegid, 1, gid_t);
//int setgid(gid_t gid);
inject(1, int, setgid, 1, gid_t);
//int setuid(uid_t uid);
inject(1, int, setuid, 1, uid_t);
//unsigned int sleep(unsigned int seconds);
inject(1, unsigned int, sleep, 1, unsigned int);
//int symlink(const char *target, const char *linkpath);
inject(1, int, symlink, 2, const char*, const char*);
//int unlink(const char *pathname);
inject(1, int, unlink, 1, const char*);
//ssize_t write(int fd, const void *buf, size_t count);
inject(1, ssize_t, write, 3, fd_t, const void*, size_t);
//int chmod(const char *pathname, mode_t mode);
inject(1, int, chmod, 2, const char*, mode_t);
//int fchmod(int fd, mode_t mode);
inject(1, int, fchmod, 2, fd_t, mode_t);
//int __xstat(int ver, const char *pathname, struct stat *buf);
inject(0, int, __xstat, 3, int, const char*, struct stat*);
//int __fxstat(int ver, int fd, struct stat *buf);
inject(0, int, __fxstat, 3, int, fd_t, struct stat*);
//int __lxstat(int ver, const char *pathname, struct stat *buf);
inject(0, int, __lxstat, 3, int, const char*, struct stat*);
//int mkdir(const char *pathname, mode_t mode);
inject(1, int, mkdir, 2, const char*, mode_t);
//int mkfifo(const char *pathname, mode_t mode);
inject(1, int, mkfifo, 2, const char*, mode_t);
//mode_t umask(mode_t mask)
inject(1, mode_t, umask, 1, mode_t);
//int fputs_unlocked(const char *s, FILE *stream);
inject(1, int, fputs_unlocked, 2, const char*, FILE*);
//int socket(int domain, int type, int protocol);
inject(1, fd_t, socket, 3, int, int, int);
//pid_t getpid(void);
inject(1, int, getpid, 0);
//char *getlogin(void);
inject(1, char*, getlogin, 0);
//int fputc_unlocked(int c, FILE *stream);
inject(1, int, fputc_unlocked, 2, int, FILE*);
inject(0, int, system, 1, const char*);

# Lab2 Library Inject

`make all` to build library
`make test` to run test

## implemeted injection
- minimal requirement
    - `closedir fdopendir opendir readdir readdir_r rewinddir seekdir telldir creat open remove rename setbuf setvbuf tempnam tmpfile tmpnam exit getenv mkdtemp mkstemp putenv rand rand_r setenv srand system chdir chown close dup dup2 _exit execl execle execlp execv execve execvp fchdir fchown fork fsync ftruncate getcwd getegid geteuid getgid getuid link pipe pread pwrite read readlink rmdir setegid seteuid setgid setuid sleep symlink unlink write chmod fchmod fstat lstat mkdir mkfifo stat umask`
    - bonus
        - `fputs_unlocked`: it may log some output
        - `fputc_unlocked`: it may log some output
        - `socket`: it may log the network communication
        - `getpid`: it may log the current process id
        - `getlogin`: it may log the login 

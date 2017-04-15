import sys


ALL_INJECT = 'closedir fdopendir opendir readdir readdir_r rewinddir seekdir telldir creat open remove rename setbuf setvbuf tempnam tmpfile tmpnam exit getenv mkdtemp mkstemp putenv rand rand_r setenv srand system chdir chown close dup dup2 _exit execl execle execlp execv execve execvp fchdir fchown fork fsync ftruncate getcwd getegid geteuid getgid getuid link pipe pread pwrite read readlink rmdir setegid seteuid setgid setuid sleep symlink unlink write chmod fchmod __fxstat __lxstat mkdir mkfifo __xstat umask fputs_unlocked fputc_unlocked socket getpid getlogin'.split()
fail, success = 0, 0
with open(sys.argv[1], 'r') as f:
    content = f.read()
    for inject in ALL_INJECT:
        if content.find(inject + '(') == -1:
            fail += 1
            print(inject, 'fail')
        else:
            success += 1
            print(inject, 'success')

print('fail: ', fail)
print('success: ', success)


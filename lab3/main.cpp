#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include "command.h"
#include "utils.h"
#include "job.h"
using namespace std;
int mysh_pid = 0, mysh_pgid = 0;
job_list_t job_list(32);

int prompt(){
    my_printf("shell> ");
    return 1;
}

void my_exit(int code){
    for(int i = 0 ; i < job_list.job_list_size ; i++){
        if(job_list.job_list_used[i]){
            kill(-job_list[i].pgid, SIGKILL);
            job_list.free_job(i);
        }
    }
    my_printf("\n");
    exit(code);
}

void zombie_handler(int sig){
	while(true){
		int status, pid = waitpid(-1, &status, WNOHANG);
		if(pid > 0 && WIFEXITED(status)){
            int job_id = job_list.find_job_id(pid);
            if(job_id != -1){
                job_list[job_id].non_exited_number--;
                if(job_list[job_id].if_exited()){
                    job_list.free_job(job_id);
                }
            }
		}else break;
	}
}
void sigint_handler(int sig){
    if(tcgetpgrp(0) == mysh_pgid){
        fflush(stdin);
        my_printf("\n");
        prompt();
    }
}
void sigtstp_handler(int sig){}
int my_exec(const command_t &command){
    const char *args[BUFFER_SIZE] = {0};
    for(int i = 0 ; i < (int)command.size() ; i++)
        args[i] = command[i].c_str();
    if(execvp(args[0], (char* const*)args) == -1){
        exit(errno);
    }
    return 0;
}
void my_wait(int job_id, int option){
    int num = job_list[job_id].non_exited_number;
    while(num--){
        int status, pid = waitpid(-job_list[job_id].pgid, &status, option);
        if(pid > 0 && WIFEXITED(status)){
            job_list[job_id].non_exited_number--;
        }
    }
}
int do_multi_commands(const command_list_t &commands){
    int p[commands.size() - 1][2], pgid = 0, pid;
    vector<int> pids;
    for(int i = 0 ; i < (int)commands.size() ; i++){
		if(i != (int)commands.size() - 1)
            pipe(p[i]);
        pids.push_back(pid = fork());
        if(pid == 0){
            if(i != 0){
                dup2(p[i - 1][0], 0);
                close(p[i - 1][0]);
                close(p[i - 1][1]);
            }
            if(i != (int)commands.size() - 1){
                dup2(p[i][1], 1);
                close(p[i][0]);
                close(p[i][1]);
            }
            if(commands[i].file_stdin != ""){
                int fd = open(commands[i].file_stdin.c_str(), O_RDONLY);
                if(fd == -1)my_printf("no such file or directory: %s\n", commands[i].file_stdin.c_str());
                dup2(fd, 0);
                close(fd);
            }
            if(commands[i].file_stdout != ""){
                int fd = open(commands[i].file_stdout.c_str(), O_WRONLY | O_CREAT, 0664);
                if(fd == -1)my_printf("no such file or directory: %s\n", commands[i].file_stdin.c_str());
                dup2(fd, 1);
                close(fd);
            }
            my_exec(commands[i]);
        }else if(pid > 0){
            setpgid(pid, pgid);
            if(i != 0) close(p[i - 1][0]), close(p[i - 1][1]);
            if(i == 0)pgid = pid;
            if(i == 0 && !commands.background)tcsetpgrp(0, pgid);
            if(!executable(commands[i][0])){
                my_printf("-mysh: %s: command not found\n", commands[i][0].c_str());
            }
        }else{
            printf("fork error\n");
			if(i != (int)commands.size() - 1) close(p[i][0]), close(p[i][1]);
            return errno;
        }
    }
    int job_id = job_list.insert_job(job_t(pgid, commands, pids));
    if(!commands.background){
        my_wait(job_id, WUNTRACED);
        if(job_list[job_id].if_exited())
            job_list.free_job(job_id);
        tcsetpgrp(0, mysh_pgid);
    }
    return 0;
}
inline int my_fg(int job_id){
    if(kill(-job_list[job_id].pgid, SIGCONT) == -1)return errno;
    tcsetpgrp(0, job_list[job_id].pgid);
    my_wait(job_id, WUNTRACED);
    tcsetpgrp(0, getpgrp());
    return 0;
}
inline int my_bg(int job_id){
    if(kill(-job_list[job_id].pgid, SIGCONT) == -1)return errno;
    return 0;
}
int my_export(const string &env){
    char *env_dup = strdup(env.c_str());
    char *key = env_dup;
    char *val = strchr(env_dup, '=');
    *(val++) = 0;
    int res = setenv(key, val, 1);
    free(env_dup);
    return res;
}
int my_unset(const string &env){
    return unsetenv(env.c_str());
}
int do_commands(const command_list_t &commands){
    if(commands.size() == 0)return 2;
    /* internal commands */
    if(commands[0][0] == "exit"){
        my_exit(0);
    }else if(commands[0][0] == "jobs"){
        job_list.print();
        return 0;
    }else if(commands[0][0] == "fg"){
        if(commands[0].size() < 2)return 2;
        return my_fg(stoi(commands[0][1]));
    }else if(commands[0][0] == "bg"){
        if(commands[0].size() < 2)return 2;
        return my_bg(stoi(commands[0][1]));
    }else if(commands[0][0] == "export"){
        if(commands[0].size() < 2)return 2;
        return my_export(commands[0][1]);
    }else if(commands[0][0] == "unset"){
        if(commands[0].size() < 2)return 2;
        return my_unset(commands[0][1]);
    }
	return do_multi_commands(commands);
}
int main(){
    mysh_pgid = getpgid(mysh_pid = getpid());
    signal(SIGCHLD, zombie_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    char raw_command[BUFFER_SIZE];
    while(prompt()){
        if(fgets(raw_command, BUFFER_SIZE, stdin) == NULL)
            my_exit(0);
        raw_command[strlen(raw_command) - 1] = 0;
        command_list_t commands(raw_command);
        do_commands(commands);
    }
    return 0;
}

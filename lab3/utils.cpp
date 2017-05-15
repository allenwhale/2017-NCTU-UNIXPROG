#include "utils.h"
#include <sys/stat.h>

int my_printf(const char *format, ...){
    va_list args;
    va_start(args, format);
    int res = vfprintf(stdout, format, args);
    va_end(args);
    fflush(stdout);
    return res;
}

bool executable(const string& str){
	char *ss = strdup(getenv("PATH")), *ptr = strtok(ss, ":");
	vector<string> path;
    if(str[0] == '.' && str[1] == '/')
        path.push_back(".");
	while(ptr){
		path.push_back(ptr);
		ptr = strtok(NULL, ":");
	}
    free(ss);
	for(auto p: path){
		string abs_path = p + "/" + str;
		struct stat s;
		if(((stat(abs_path.c_str(), &s) >= 0) && (s.st_mode > 0) && (S_IEXEC & s.st_mode)))
			return true;
	}
	return false;
}

string get_state(int pid){
    string proc = "/proc/" + to_string(pid) + "/status";
    FILE *f = fopen(proc.c_str(), "r");
    if(!f)return "Exited";
    static char buf[BUFFER_SIZE];
    while(fgets(buf, BUFFER_SIZE, f)){
        if(!strncmp(buf, "State:", 6)){
            buf[strlen(buf) - 1] = 0;
            return string(buf + 7);
        }
    }
    return "unknown";
}

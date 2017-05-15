#include "command.h"
#include "utils.h"
#include <ctype.h>
#include <glob.h>

command_t::command_t(){
    clear();
    file_stdin = file_stdout = "";
}

command_t::command_t(const string &s){
    clear();
    file_stdin = file_stdout = "";
    string buf = "";
    vector<string> raw;
    for(int i = 0 ; i < (int)s.size() ; i++){
        if(isspace(s[i])){
            if(buf != ""){
                raw.push_back(buf);
                buf = "";
            }
        }else if(s[i] == '<' || s[i] == '>'){
            if(buf != ""){
                raw.push_back(buf);
                buf = "";
            }
            char rdr = s[i++];
            while(i < (int)s.size() && isspace(s[i])){
                i++;
            }
            while(i < (int)s.size() && !isspace(s[i]) && s[i] != '<' && s[i] != '>'){
                buf += s[i++];
            }
            i--;
            if(rdr == '<'){
                file_stdin = buf;
            }else if(rdr == '>'){
                file_stdout= buf;
            }
            buf = "";
        }else{
            buf += s[i];
        }
    }
    if(buf != "")
        raw.push_back(buf);
    for(auto arg : raw){
        if(arg.find("*") != string::npos || arg.find("?") != string::npos){
            glob_t glob_res;
            glob(arg.c_str(), GLOB_TILDE, NULL, &glob_res);
            for(int i = 0 ; i < (int)glob_res.gl_pathc ; i++){
                push_back(glob_res.gl_pathv[i]);
            }
            if(!glob_res.gl_pathc)
                push_back(arg);
        }else{
            push_back(arg);
        }
    }
}

void command_t::print(const string &end) const {
    for(auto s : *(this))
        my_printf("%s ", s.c_str());
    if(file_stdin != "")
        my_printf("< %s ", file_stdin.c_str());
    if(file_stdout != "")
        my_printf("> %s ", file_stdout.c_str());
    my_printf("%s", end.c_str());
}

command_list_t::command_list_t(){
    clear();
    background = 0;
}

command_list_t::command_list_t(const string &s){
    clear();
    background = 0;
    char *raw = strdup(s.c_str());
    int len = s.size();
    while(len && isspace(raw[len - 1])){
        raw[--len] = 0;
    }
    if(raw[len - 1] == '&'){
        background = 1;
        raw[--len] = 0;
    }
    char *ptr = strtok(raw, "|");
    while(ptr){
        push_back(command_t(ptr));
        ptr = strtok(NULL, "|");
    }
    free(raw);
}

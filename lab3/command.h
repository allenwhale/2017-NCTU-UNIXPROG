#ifndef __COMMAND__H__
#define __COMMAND__H__

#include <bits/stdc++.h>
using namespace std;

class command_t: public vector<string> {
    public:
        string file_stdin, file_stdout;
        command_t();
        command_t(const string&);
        void print(const string &end="\n") const;
};

class command_list_t: public vector<command_t> {
    public:
        int background;
        command_list_t();
        command_list_t(const string&);
};


#endif

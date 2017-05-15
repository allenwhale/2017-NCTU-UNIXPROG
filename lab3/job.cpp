#include "job.h"
#include "utils.h"

job_t::job_t(){
    job_id = 0;
    commands.clear();
    pids.clear();
}

job_t::job_t(int _pgid, const command_list_t &_commands, const vector<int> &_pids){
    job_id = 0;
    pgid = _pgid;
    commands = _commands;
    pids = _pids;
    non_exited_number = pids.size();
}

bool job_t::if_exited() const {
    return non_exited_number == 0;
}

void job_t::print() const {
    char buf[BUFFER_SIZE];
    sprintf(buf, "[%d]", job_id);
    for(int i = 0 ; i < (int)commands.size() ; i++){
        if(i == 0)my_printf("%-10s", buf);
        else my_printf("%-10s", "");
        my_printf("%-20s", get_state(pids[i]).c_str());
        commands[i].print("");
        if(i != (int)commands.size() - 1)my_printf("|");
        my_printf("\n");
    }
}

job_list_t::job_list_t(){}

job_list_t::job_list_t(int size){
    job_list_size = size;
    job_list.resize(job_list_size);
    job_list_used.resize(job_list_size);
}

int job_list_t::insert_job(const job_t &job){
    for(int i = 0 ; i < job_list_size ; i++){
        if(!job_list_used[i]){
            job_list_used[i] = true;
            job_list[i] = job;
            job_list[i].job_id = i;
            return i;
        }
    }
    return -1;
}

void job_list_t::free_job(int job_id){
    job_list_used[job_id] = false;
}

int job_list_t::find_job_id(int pid) const {
    for(int i = 0 ; i < job_list_size ; i++){
        if(job_list_used[i]){
            for(auto p : job_list[i].pids){
                if(p == pid)return i;
            }
        }
    }
    return -1;
}

void job_list_t::print() const {
    for(int i = 0 ; i < job_list_size ; i++){
        if(job_list_used[i]){
            job_list[i].print();
        }
    }
}

job_t& job_list_t::operator [] (int n){
    return job_list[n];
}

const job_t& job_list_t::operator [] (int n) const {
    return job_list[n];
}

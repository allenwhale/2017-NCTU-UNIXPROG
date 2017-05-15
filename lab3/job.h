#ifndef __JOB__H__
#define __JOB__H__

#include <bits/stdc++.h>
#include "command.h"
using namespace std;

struct job_t{
    job_t();
    job_t(int, const command_list_t&, const vector<int>&);
    int job_id, pgid, non_exited_number;
    command_list_t commands;
    vector<int> pids;
    bool if_exited() const;
    void print() const;
};

struct job_list_t{
    job_list_t();
    job_list_t(int);
    vector<job_t> job_list;
    vector<bool> job_list_used;
    int job_list_size;
    int insert_job(const job_t&);
    void free_job(int);
    int find_job_id(int) const;
    void print() const;
    job_t& operator [] (int);
    const job_t& operator [] (int) const;
};

#endif

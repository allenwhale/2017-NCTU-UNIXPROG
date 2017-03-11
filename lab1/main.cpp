#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex>
#include <arpa/inet.h>
using namespace std;
using namespace __gnu_pbds;
struct program {
    int pid;
    string cmdline;
    string to_string() const {
        if(pid == 0) return "-";
        return ::to_string(pid) + "/" + cmdline;
    }
};
int flag_tcp = 0, flag_udp = 0;
cc_hash_table<int, program> inode_to_program;
regex filter_string_regex;
string format_addr_port(const string &address, int port) {
    char ip[256];
    if(address.size() > 8) {
        //ipv6
        struct sockaddr_in6 addr;
        sscanf(address.c_str(),
               "%08X%08X%08X%08X",
               &addr.sin6_addr.s6_addr32[0],
               &addr.sin6_addr.s6_addr32[1],
               &addr.sin6_addr.s6_addr32[2],
               &addr.sin6_addr.s6_addr32[3]);
        inet_ntop(AF_INET6, &addr.sin6_addr, ip, sizeof(ip));
    } else {
        //ipv4
        struct sockaddr_in addr;
        sscanf(address.c_str(), "%X", &addr.sin_addr.s_addr);
        inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    }
    return string(ip) + ":" + (port ? to_string(port) : "*");
}
void parse_socket(const string &proto, const string &proc_filename) {
    FILE *f = fopen(proc_filename.c_str(), "r");
    unsigned int local_port, remote_port, inode;
    char buf[256], local_addr[128], remote_addr[128];
    fgets(buf, 256, f);
    while(fgets(buf, 256, f)) {
        sscanf(buf, "%*d: %64[a-zA-Z0-9]:%X %64[a-zA-Z0-9]:%X %*X %*lX:%*lX %*X:%*lX %*lX %*d %*d %u %*s\n",
               local_addr, &local_port, remote_addr, &remote_port, &inode);
        if(!regex_search(inode_to_program[inode].cmdline, filter_string_regex))continue;
        printf("%-6s%-24s%-24s%s\n",
               proto.c_str(),
               format_addr_port(local_addr, local_port).c_str(),
               format_addr_port(remote_addr, remote_port).c_str(),
               inode_to_program[inode].to_string().c_str());
    }
}

bool isnumber(const string &s) {
    return !s.empty() && all_of(s.begin(), s.end(), ::isdigit);
}

void parse_proc_fd() {
    DIR *dir = opendir("/proc");
    vector<int> pids;
    while(struct dirent *dirp = readdir(dir)) {
        if(isnumber(dirp->d_name)) {
            pids.push_back(stoi(dirp->d_name));
        }
    }
    closedir(dir);
    for(int pid : pids) {
        string fd_dir = "/proc/" + to_string(pid) + "/fd";
        dir = opendir(fd_dir.c_str());
        if(!dir) continue;
        while(struct dirent *dirp = readdir(dir)) {
            if(isnumber(dirp->d_name)) {
                int fd = stoi(dirp->d_name);
                char linkname[256], cmdline[1024];
                int linkname_len = readlink(("/proc/" + to_string(pid) + "/fd/" + to_string(fd)).c_str(), linkname, 256);
                int inode;
                if(linkname_len == -1) continue;
                linkname[linkname_len] = 0;
                if(sscanf(linkname, "socket:[%d]", &inode) != 1 && sscanf(linkname, "[0000]:%d", &inode) != 1) continue;
                int cmd_fd = open(("/proc/" + to_string(pid) + "/cmdline").c_str(), O_RDONLY);
                if(cmd_fd == -1)continue;
                int cmdline_len = read(cmd_fd, cmdline, 1024);
                if(cmdline_len == -1)continue;
                cmdline[cmdline_len] = 0;
                for(int i=0; i<cmdline_len; i++)
                    if(cmdline[i] == 0) cmdline[i] = ' ';
                inode_to_program[inode] = { pid, cmdline };
            }
        }
        closedir(dir);
    }
}
void print_header() {
    printf("%-6s%-24s%-24s%s\n", "Proto", "Local Address", "Foreign Address", "PID/Program name and arguments");
}

int main(int argc, char* const* argv) {
    const char *short_options = "tu";
    struct option long_options[] = {
        {"tcp", no_argument, 0, 't'},
        {"udp", no_argument, 0, 'u'},
        {0, 0, 0, 0}
    };
    while(1) {
        int c = getopt_long(argc, argv, short_options, long_options, NULL);
        if(c == -1)break;
        switch(c) {
        case 't':
            flag_tcp = 1;
            break;
        case 'u':
            flag_udp = 1;
            break;
        default:
            break;
        }
    }
    if(argv[optind]){
        filter_string_regex = regex(argv[optind]);
    }else{
        filter_string_regex = regex("");
    }
    parse_proc_fd();
    if(!flag_tcp && !flag_udp)
        flag_tcp = flag_udp = 1;
    if(flag_tcp) {
        printf("List of TCP connections:\n");
        print_header();
        parse_socket("tcp", "/proc/net/tcp");
        parse_socket("tcp6", "/proc/net/tcp6");
    }
    if(flag_udp) {
        if(flag_tcp)puts("");
        printf("List of UDP connections:\n");
        print_header();
        parse_socket("udp", "/proc/net/udp");
        parse_socket("udp6", "/proc/net/udp6");
    }
    return 0;
}

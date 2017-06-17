#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ncurses.h>
#include <errno.h>
#include "board.h"
#include "screen.h"
using namespace std;

pair<int, int> recv_step(int fd) {
    char buf[1024];
    if(read(fd, buf, 1024) == 0) return {-1, -1};
    pair<int, int> res;
    stringstream ss(buf);
    ss >> res.first;
    ss >> res.second;
    return res;
}

int send_step(int fd, int y, int x) {
    char buf[1024] = {0};
    sprintf(buf, "%d %d", y, x);
    int len = strlen(buf);
    if(write(fd, buf, len) != len) return -1;
    return 0;
}

void handler(int player, int sock_fd) {
    init_screen();
    int max_fd = sock_fd + 1;
    fd_set rfds, n_rfds;
    FD_ZERO(&n_rfds);
    FD_SET(sock_fd, &n_rfds);
    FD_SET(STDIN_FILENO, &n_rfds);
    move(height - 1, 0);
    attron(A_BOLD);
    printw("Arrow keys: move; Space/Enter: put; Q: quit");
    attroff(A_BOLD);
    Board board;
    board.show();
    int now_player = PLAYER(0);
    int stopped = false, ended = false;
    while(1){
        if(!ended){
            if(player == now_player)
                board.show_msg(0, "Player #%d: It\'s my turn", PLAYER_ID(player) + 1);
            else
                board.show_msg(1, "Player #%d: Waiting for peer", PLAYER_ID(player) + 1);
        }
        memcpy(&rfds, &n_rfds, sizeof(rfds));
        if(select(max_fd, &rfds, NULL, NULL, NULL) < 0){
            break;
        }
        if(FD_ISSET(0, &rfds)){
            int ch = getch();
            switch(ch){
                case 'r':
                case 'R':
                    board = Board();
                    board.show();
                    stopped = ended = false;
                    send_step(sock_fd, -2, -2);
                    now_player = PLAYER(0);
                    break;
                case 'q':
                case 'Q':
                    stopped = true;
                    break;
                case KEY_UP:
                    board.set_cursor((board.cursor_y + Board::BOARD_SIZE - 1) % Board::BOARD_SIZE, board.cursor_x);
                    break;
                case KEY_DOWN:
                    board.set_cursor((board.cursor_y + 1) % Board::BOARD_SIZE, board.cursor_x);
                    break;
                case KEY_LEFT:
                    board.set_cursor(board.cursor_y, (board.cursor_x + Board::BOARD_SIZE - 1) % Board::BOARD_SIZE);
                    break;
                case KEY_RIGHT:
                    board.set_cursor(board.cursor_y, (board.cursor_x + 1) % Board::BOARD_SIZE);
                    break;
                case KEY_ENTER:
                case '\n':
                case ' ':
                    if(ended || now_player != player) break;
                    if(!board.is_valid(player, board.cursor_y, board.cursor_x)){
                        board.show_msg(1, "Invalid place");
                        break;
                    }
                    board.set(player, board.cursor_y, board.cursor_x);
                    if(send_step(sock_fd, board.cursor_y, board.cursor_x) != 0) stopped = true;
                    if(!board.is_over(OPPSITE(player)))now_player = OPPSITE(player);
                    break;
            }
            if(stopped) break;
        }
        if(FD_ISSET(sock_fd, &rfds)){
            auto res = recv_step(sock_fd);
            if(res == pair<int, int>{-1, -1}){
                break;
            }
            if(res == pair<int, int>{-2, -2}){
                now_player = PLAYER(0);
                board = Board();
                board.show();
            }else if(!ended && now_player == OPPSITE(player)){
                board.set(OPPSITE(player), res.first, res.second);
                if(!board.is_over(player))now_player = player;
            }
        }
        if(board.is_over(PLAYER(0)) && board.is_over(PLAYER(1))){
            auto score = board.score();
            if(score[PLAYER_ID(player)] > score[PLAYER_ID(OPPSITE(player))]) board.show_msg(0, "Player #%d: You win", PLAYER_ID(player));
            else board.show_msg(0, "Player #%d: You lose", PLAYER_ID(player));
            ended = true;
        }
    }
    end_screen();
}

void server_mode(int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0){
        printf("Create socket error\n");
        return;
    }
    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));
    sockaddr_in dest;
    memset(&dest, 0 , sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = INADDR_ANY;
    if(bind(sock_fd, (sockaddr*)&dest, sizeof(dest)) < 0){
        printf("Bind error\n");
        return;
    }
    if(listen(sock_fd, 5) < 0){
        printf("Listen error\n");
        return;
    }
    sockaddr_in client_addr;
    int addr_len = sizeof(sockaddr_in);
    int client_fd;
    printf("Waiting for a client on port %d\n", port);
    client_fd = accept(sock_fd, (sockaddr*)&client_addr, (socklen_t*)&addr_len);
    if(client_fd < 0){
        printf("Accept error\n");
    }
    handler(PLAYER(0), client_fd);
    close(sock_fd);
    close(client_fd);
}

void client_mode(const char *server) {
    struct hostent *he;
    char *srv = strdup(server);
    char *port = strchr(srv, ':');
    if(port == NULL){
        free(srv);
        printf("Argument invalid\n");
        return;
    }
    *port++ = 0;
    he = gethostbyname(srv);
    free(srv);
    if(he == NULL){
        printf("Hostname not found\n");
        return;
    }
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(stoi(port));
    memcpy(&(dest.sin_addr), he->h_addr, sizeof(dest.sin_addr));
    if(connect(sock_fd, (sockaddr*)&dest, sizeof(dest)) != 0){
        printf("Connect error\n");
        return;
    }
    handler(PLAYER(1), sock_fd);
    close(sock_fd);
}

int main(int argc, char **argv){
    if(!strcmp(argv[1], "-s")){
        server_mode(stoi(argv[2]));
    }else if(!strcmp(argv[1], "-c")){
        client_mode(argv[2]);
    }
    return 0;
}

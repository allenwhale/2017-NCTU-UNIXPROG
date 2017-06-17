#ifndef __BOARD__H__
#define __BOARD__H__

#include <bits/stdc++.h>
#include <ncurses.h>
using namespace std;

#define PLAYER_SYMS ("OX")
#define PLAYER_SYM(x) (PLAYER_SYMS[(x)])
#define PLAYER(x) (vector<int>{1, -1}[(x)])
#define PLAYER_ID(x) ((x) == 1 ? 0 : 1)
#define OPPSITE(x) (-(x))

struct Board {
    static const int BOARD_SIZE = 8;
    const int TOP = 1;
    const int LEFT = 2;
    const int BOX_HEIGHT = 2;
    const int BOX_WIDTH = 3;
    array<array<int, Board::BOARD_SIZE>, Board::BOARD_SIZE> b;
    int cursor_y, cursor_x;
    Board(int cy = 3, int cx = 3);
    Board(const Board &);
    Board& operator = (const Board &);
    chtype sym_with_color(int, int) const;
    void set_cursor(int, int);
    void show_box(int, int, int, int) const;
    void show_cursor(int, int, int) const;
    void show() const;
    void set(int, int, int);
    array<int, 2> score() const;
    void show_score() const;
    void show_msg(int, const char *, ...) const;
    bool is_over(int) const;
    bool is_valid(int, int, int) const;
    static bool in_board(int, int);
    array<int, Board::BOARD_SIZE>& operator [] (int);
    const array<int, Board::BOARD_SIZE>& operator [] (int) const;
};

#endif

#include "screen.h"
#include <ncurses.h>

int height, width;

void init_screen() {
    initscr();
    getmaxyx(stdscr, height, width);
    raw();
    cbreak();
    halfdelay(1);
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    init_colors();
    clear();
}

void init_colors() {
    start_color();
    init_pair(PLAYER_COLOR(0), COLOR_BLACK, COLOR_GREEN);
    init_pair(PLAYER_COLOR(1), COLOR_BLACK, COLOR_MAGENTA);
    init_pair(BORDER_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(CURSOR_COLOR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(WARN_MSG_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(OK_MSG_COLOR, COLOR_GREEN, COLOR_BLACK);
}

void end_screen() {
    endwin();
}

#include "board.h"
#include "screen.h"

Board::Board(int cy, int cx) {
    cursor_y = -1;
    cursor_x = -1;
    for(int i = 0 ; i < Board::BOARD_SIZE ; i++) {
        for(int j = 0 ; j < Board::BOARD_SIZE ; j++) {
            b[i][j] = 0;
        }
    }
    int t = Board::BOARD_SIZE / 2 - 1;
    b[t][t] = b[t + 1][t + 1] = PLAYER(0);
    b[t][t + 1] = b[t + 1][t] = PLAYER(1);
    set_cursor(cy, cx);
}

Board::Board(const Board &n) {
    b = n.b;
    cursor_x = n.cursor_x;
    cursor_y = n.cursor_y;
}
Board& Board::operator = (const Board &n) {
    b = n.b;
    cursor_x = n.cursor_x;
    cursor_y = n.cursor_y;
    return *this;
}

chtype Board::sym_with_color(int y, int x) const {
    if(b[y][x] == PLAYER(0)) return PLAYER_SYM(0) | COLOR_PAIR(PLAYER_COLOR(0));
    if(b[y][x] == PLAYER(1)) return PLAYER_SYM(1) | COLOR_PAIR(PLAYER_COLOR(1));
    return ' ';
}

void Board::set_cursor(int cy, int cx) {
    if(cursor_y != -1 && cursor_x != -1) show_cursor(cursor_y, cursor_x, 0);
    cursor_y = cy;
    cursor_x = cx;
    show_cursor(cursor_y, cursor_x, 1);
}

void Board::show_box(int y, int x, int border_color, int highlight) const {
    attron(highlight ? A_BOLD : A_NORMAL);
    attron(COLOR_PAIR(border_color));

    move(Board::TOP + y * Board::BOX_HEIGHT + 0, Board::LEFT + x * (Board::BOX_WIDTH + 1));
    if(y == 0) addch(x == 0 ? ACS_ULCORNER : ACS_TTEE);
    else addch(x == 0 ? ACS_LTEE : ACS_PLUS);
    for(int i = 0 ; i < Board::BOX_WIDTH ; i++)
        addch(ACS_HLINE);
    if(y == 0) addch(x == Board::BOARD_SIZE - 1 ? ACS_URCORNER : ACS_TTEE);
    else addch(x == Board::BOARD_SIZE - 1 ? ACS_RTEE : ACS_PLUS);

    move(Board::TOP + y * Board::BOX_HEIGHT + 1, Board::LEFT + x * (Board::BOX_WIDTH + 1));
    addch(ACS_VLINE);
    for(int i = 0 ; i < Board::BOX_WIDTH / 2 ; i++)
        addch(' ');
    addch(sym_with_color(y, x));
    for(int i = 0 ; i < Board::BOX_WIDTH / 2 ; i++)
        addch(' ');
    addch(ACS_VLINE);

    move(Board::TOP + y * Board::BOX_HEIGHT + 2, Board::LEFT + x * (Board::BOX_WIDTH + 1));
    if(y == Board::BOARD_SIZE - 1) addch(x == 0 ? ACS_LLCORNER : ACS_BTEE);
    else addch(x == 0 ? ACS_LTEE : ACS_PLUS);
    for(int i = 0 ; i < Board::BOX_WIDTH ; i++)
        addch(ACS_HLINE);
    if(y == Board::BOARD_SIZE - 1) addch(x == Board::BOARD_SIZE - 1 ? ACS_LRCORNER : ACS_BTEE);
    else addch(x == Board::BOARD_SIZE - 1 ? ACS_RTEE : ACS_PLUS);
    attroff(COLOR_PAIR(border_color));
    attroff(highlight ? A_BOLD : A_NORMAL);
}

void Board::show() const {
    for(int i = 0 ; i < Board::BOARD_SIZE ; i++) {
        for(int j = 0 ; j < Board::BOARD_SIZE ; j++) {
            show_box(j, i, BORDER_COLOR, 0);
        }
    }
    show_cursor(cursor_y, cursor_x, 1);
    show_score();
    refresh();
}

void Board::show_cursor(int y, int x, int show) const {
    show_box(y, x, show ? CURSOR_COLOR : BORDER_COLOR, show);
    refresh();
}

array<int, Board::BOARD_SIZE>& Board::operator [] (int n) {
    return b[n];
}

const array<int, Board::BOARD_SIZE>& Board::operator [] (int n) const {
    return b[n];
}

array<int, 2> Board::score() const {
    array<int, 2> res;
    res[0] = res[1] = 0;
    for(int i = 0 ; i < Board::BOARD_SIZE ; i++) {
        for(int j = 0 ; j < Board::BOARD_SIZE ; j++) {
            res[0] += (b[i][j] == PLAYER(0));
            res[1] += (b[i][j] == PLAYER(1));
        }
    }
    return res;
}

void Board::show_score() const {
    auto now_score = score();
    for(int i = 0 ; i < 2 ; i++) {
        move(Board::TOP + 3 + i * 2, Board::LEFT + 4 * Board::BOARD_SIZE + 10);
        printw("Player #%d ", i + 1);
        addch(PLAYER_SYM(i) | COLOR_PAIR(PLAYER_COLOR(i)));
        printw(" : %d   ", now_score[i]);
    }
    refresh();
}

void Board::show_msg(int warn, const char *msg, ...) const {
    move(0, 0);
    printw("%-30s", "");
    move(0, 0);
    attron(warn ? A_BLINK : A_NORMAL);
    attron(warn ? COLOR_PAIR(WARN_MSG_COLOR) : COLOR_PAIR(OK_MSG_COLOR));
    va_list args;
    va_start(args, msg);
    vw_printw(stdscr, msg, args);
    va_end(args);
    attroff(warn ? COLOR_PAIR(WARN_MSG_COLOR) : COLOR_PAIR(OK_MSG_COLOR));
    attroff(warn ? A_BLINK : A_NORMAL);
    refresh();
}

bool Board::in_board(int y, int x) {
    return y >= 0 && y < Board::BOARD_SIZE && x >= 0 && x < Board::BOARD_SIZE;
}

bool Board::is_valid(int player, int y, int x) const {
    static int dx[] = {0, 0, 1, 1, 1, -1, -1, -1};
    static int dy[] = {1, -1, 1, -1, 0, 1, -1, 0};
    if(b[y][x] != 0) return false;
    for(int k = 0 ; k < 8 ; k++) {
        int cy = y, cx = x, cnt = 0;
        do {
            cy += dy[k], cx += dx[k];
            cnt++;
        }while(Board::in_board(cy, cx) && b[cy][cx] && b[cy][cx] != player);
        if(Board::in_board(cy, cx) && b[cy][cx] == player && cnt > 1) 
            return true;
    }
    return false;
}

bool Board::is_over(int player) const {
    for(int i = 0 ; i < Board::BOARD_SIZE ; i++) {
        for(int j = 0 ; j < Board::BOARD_SIZE ; j++) {
            if(is_valid(player, i, j))
                return false;
        }
    }
    return true;
}

void Board::set(int player, int y, int x) {
    static int dx[] = {0, 0, 1, 1, 1, -1, -1, -1};
    static int dy[] = {1, -1, 1, -1, 0, 1, -1, 0};
    for(int k = 0 ; k < 8 ; k++) {
        int cy = y, cx = x, cnt = 0;
        do {
            cy += dy[k], cx += dx[k];
            cnt++;
        }while(Board::in_board(cy, cx) && b[cy][cx] && b[cy][cx] != player);
        if(Board::in_board(cy, cx) && b[cy][cx] == player && cnt > 1) {
            while(cy != y || cx != x) {
                b[cy][cx] = player;
                cx -= dx[k], cy -= dy[k];
            }
        }
    }
    b[y][x] = player;
    show();
}

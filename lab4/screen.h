#ifndef __SCREEN__H__
#define __SCREEN__H__

#define PLAYER_COLOR(x) ((x) + 1)
#define BORDER_COLOR (PLAYER_COLOR(1) + 1)
#define CURSOR_COLOR (BORDER_COLOR + 1)
#define WARN_MSG_COLOR (CURSOR_COLOR + 1)
#define OK_MSG_COLOR (WARN_MSG_COLOR + 1)

extern int height, width;

void init_screen();
void init_colors();
void end_screen();

#endif

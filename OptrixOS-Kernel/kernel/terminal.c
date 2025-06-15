#include "terminal.h"
#include "graphics.h"
#include <stddef.h>

#define TERM_COLS 40
#define TERM_ROWS 25
#define CHAR_W 8
#define CHAR_H 8

static size_t term_row = 1;
static size_t term_col = 1;

void terminal_initialize(void) {
    graphics_init();
    graphics_clear(0);
    graphics_draw_rect(0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,15);
    term_row = 1;
    term_col = 1;
}

void terminal_clear(void) {
    graphics_draw_rect(1,1,SCREEN_WIDTH-2,SCREEN_HEIGHT-2,0);
    term_row = 1;
    term_col = 1;
}

static void newline(void) {
    term_col = 1;
    if (++term_row >= TERM_ROWS-1) {
        terminal_clear();
    }
}

static void putchar_at(char c, size_t col, size_t row) {
    graphics_draw_char(col*CHAR_W, row*CHAR_H, c, 10);
}

void terminal_write(const char* s) {
    while (*s) {
        char c = *s++;
        if (c == '\n') {
            newline();
        } else {
            putchar_at(c, term_col, term_row);
            if (++term_col >= TERM_COLS-1)
                newline();
        }
    }
}

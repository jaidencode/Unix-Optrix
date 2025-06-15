#include "terminal.h"
#include <stddef.h>

#define VGA_TEXT_MODE  ((volatile unsigned short*)0xB8000)
static size_t term_row = 0;
static size_t term_col = 0;
static const unsigned short term_color = 0x0F00; // black bg, white fg

void terminal_initialize(void) {
    term_row = 0;
    term_col = 0;
    terminal_clear();
}

void terminal_clear(void) {
    for (size_t y = 0; y < 25; ++y) {
        for (size_t x = 0; x < 80; ++x) {
            VGA_TEXT_MODE[y * 80 + x] = term_color | ' ';
        }
    }
}

static void putchar_at(char c, size_t x, size_t y) {
    VGA_TEXT_MODE[y * 80 + x] = term_color | (unsigned short)c;
}

static void newline(void) {
    term_col = 0;
    if (++term_row >= 25) {
        term_row = 24;
        for (size_t y = 1; y < 25; ++y)
            for (size_t x = 0; x < 80; ++x)
                VGA_TEXT_MODE[(y-1)*80 + x] = VGA_TEXT_MODE[y*80 + x];
        for (size_t x = 0; x < 80; ++x)
            VGA_TEXT_MODE[24*80 + x] = term_color | ' ';
    }
}

void terminal_write(const char* s) {
    while (*s) {
        char c = *s++;
        if (c == '\n') {
            newline();
        } else {
            putchar_at(c, term_col, term_row);
            if (++term_col >= 80) {
                newline();
            }
        }
    }
}

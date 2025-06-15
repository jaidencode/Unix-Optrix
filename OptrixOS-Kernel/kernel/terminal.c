#include "terminal.h"
#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t*)0xB8000)

static size_t term_row = 0;
static size_t term_col = 0;
static uint8_t term_color = 0x07;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)color << 8 | (uint8_t)c;
}

void terminal_initialize(void) {
    term_row = 0;
    term_col = 0;
    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            VGA_MEMORY[y * VGA_WIDTH + x] = vga_entry(' ', term_color);
}

void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            VGA_MEMORY[y * VGA_WIDTH + x] = vga_entry(' ', term_color);
    term_row = 0;
    term_col = 0;
}

static void newline(void) {
    term_col = 0;
    if (++term_row >= VGA_HEIGHT) {
        terminal_clear();
    }
}

static void putchar_at(char c, size_t col, size_t row) {
    VGA_MEMORY[row * VGA_WIDTH + col] = vga_entry(c, term_color);
}

void terminal_write(const char* s) {
    while (*s) {
        char c = *s++;
        if (c == '\n') {
            newline();
        } else {
            putchar_at(c, term_col, term_row);
            if (++term_col >= VGA_WIDTH)
                newline();
        }
    }
}

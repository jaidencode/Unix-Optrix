#include "screen.h"
#include "ports.h"
#include <stdint.h>

static volatile uint16_t *vga = (uint16_t*)0xB8000;

void screen_init(void) {
    screen_clear();
}

void screen_clear(void) {
    for(int y=0; y<SCREEN_ROWS; y++)
        for(int x=0; x<SCREEN_COLS; x++)
            vga[y*SCREEN_COLS + x] = (BACKGROUND_COLOR << 8) | ' ';
}

void screen_put_char(int col, int row, char c, uint8_t color) {
    if(col < 0 || col >= SCREEN_COLS || row < 0 || row >= SCREEN_ROWS)
        return;
    vga[row*SCREEN_COLS + col] = ((uint16_t)color << 8) | (uint8_t)c;
}

void screen_put_char_offset(int col, int row, char c, uint8_t color,
                            int off_x, int off_y) {
    screen_put_char(col + off_x, row + off_y, c, color);
}

void screen_set_cursor(int col, int row) {
    if(col < 0) col = 0;
    if(col >= SCREEN_COLS) col = SCREEN_COLS - 1;
    if(row < 0) row = 0;
    if(row >= SCREEN_ROWS) row = SCREEN_ROWS - 1;
    uint16_t pos = row * SCREEN_COLS + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

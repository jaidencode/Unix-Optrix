#include "screen.h"
#include "graphics.h"
#include "font/font8x8_basic.h"
#include <stdint.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define CHAR_WIDTH 21
#define CHAR_HEIGHT 21
#define OFFSET_X 8
#define OFFSET_Y 8

void screen_clear(void) {
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00);
}

static void draw_border(void) {
    for(int x=0; x<SCREEN_WIDTH; x++) {
        put_pixel(x, 0, 0x0F);
        put_pixel(x, SCREEN_HEIGHT-1, 0x0F);
    }
    for(int y=0; y<SCREEN_HEIGHT; y++) {
        put_pixel(0, y, 0x0F);
        put_pixel(SCREEN_WIDTH-1, y, 0x0F);
    }
}

void screen_put_char(int col, int row, char c, uint8_t color) {
    if(c < 0) c = '?';
    const uint8_t *glyph = font8x8_basic[(unsigned char)c];
    int x = OFFSET_X + col * CHAR_WIDTH;
    int y = OFFSET_Y + row * CHAR_HEIGHT;
    for(int cy=0; cy<CHAR_HEIGHT; cy++) {
        uint8_t line = glyph[(cy * 8) / CHAR_HEIGHT];
        for(int cx=0; cx<CHAR_WIDTH; cx++) {
            if(line & (1 << ((cx * 8) / CHAR_WIDTH)))
                put_pixel(x+cx, y+cy, color);
            else
                put_pixel(x+cx, y+cy, 0x00);
        }
    }
}

void screen_init(void) {
    screen_clear();
    draw_border();
}

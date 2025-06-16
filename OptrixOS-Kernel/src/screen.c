#include "screen.h"
#include "graphics.h"
#include "font/font8x8_basic.h"
#include <stdint.h>

int CHAR_WIDTH = BASE_CHAR_SIZE;
int CHAR_HEIGHT = BASE_CHAR_SIZE;
int font_scale = 1;
int SCREEN_COLS = 0;
int SCREEN_ROWS = 0;

void screen_clear(void) {
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BACKGROUND_COLOR);
}

void screen_update_metrics(void) {
    CHAR_WIDTH = BASE_CHAR_SIZE * font_scale;
    CHAR_HEIGHT = BASE_CHAR_SIZE * font_scale;
    if(CHAR_WIDTH <= 0) CHAR_WIDTH = 1;
    if(CHAR_HEIGHT <= 0) CHAR_HEIGHT = 1;
    SCREEN_COLS = (SCREEN_WIDTH - 2*OFFSET_X) / CHAR_WIDTH;
    SCREEN_ROWS = (SCREEN_HEIGHT - 2*OFFSET_Y) / CHAR_HEIGHT;
}

void screen_adjust_font_scale(int delta) {
    font_scale += delta;
    if(font_scale < 1) font_scale = 1;
    if(font_scale > 5) font_scale = 5;
    screen_update_metrics();
    screen_clear();
}

static void draw_border(void) {
    /* Fancy white border with a grey inner line */
    const uint8_t outer = 0x0F; /* bright white */
    const uint8_t inner = 0x08; /* light grey  */
    for(int x=0; x<SCREEN_WIDTH; x++) {
        put_pixel(x, 0, outer);
        put_pixel(x, 1, inner);
        put_pixel(x, SCREEN_HEIGHT-2, inner);
        put_pixel(x, SCREEN_HEIGHT-1, outer);
    }
    for(int y=0; y<SCREEN_HEIGHT; y++) {
        put_pixel(0, y, outer);
        put_pixel(1, y, inner);
        put_pixel(SCREEN_WIDTH-2, y, inner);
        put_pixel(SCREEN_WIDTH-1, y, outer);
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
                put_pixel(x+cx, y+cy, BACKGROUND_COLOR);
        }
    }
}

void screen_put_char_offset(int col, int row, char c, uint8_t color,
                            int off_x, int off_y) {
    if(c < 0) c = '?';
    const uint8_t *glyph = font8x8_basic[(unsigned char)c];
    int x = off_x + col * CHAR_WIDTH;
    int y = off_y + row * CHAR_HEIGHT;
    for(int cy=0; cy<CHAR_HEIGHT; cy++) {
        uint8_t line = glyph[(cy * 8) / CHAR_HEIGHT];
        for(int cx=0; cx<CHAR_WIDTH; cx++) {
            if(line & (1 << ((cx * 8) / CHAR_WIDTH)))
                put_pixel(x+cx, y+cy, color);
            else
                put_pixel(x+cx, y+cy, BACKGROUND_COLOR);
        }
    }
}

void screen_init(void) {
    screen_update_metrics();
    screen_clear();
}

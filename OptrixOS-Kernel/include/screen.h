#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8
#define OFFSET_X 8
#define OFFSET_Y 8
#define SCREEN_COLS ((SCREEN_WIDTH - 2*OFFSET_X) / CHAR_WIDTH)
#define SCREEN_ROWS ((SCREEN_HEIGHT - 2*OFFSET_Y) / CHAR_HEIGHT)

#define BACKGROUND_COLOR 0x00

void screen_init(void);
void screen_clear(void);
void screen_put_char(int col, int row, char c, uint8_t color);

#endif

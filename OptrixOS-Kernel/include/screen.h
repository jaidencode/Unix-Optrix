#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define SCREEN_COLS 80
#define SCREEN_ROWS 25
#define BACKGROUND_COLOR 0x1F

void screen_init(void);
void screen_clear(void);
void screen_put_char(int col, int row, char c, uint8_t color);
void screen_put_char_offset(int col, int row, char c, uint8_t color,
                            int off_x, int off_y);
void screen_set_cursor(int col, int row);

#endif

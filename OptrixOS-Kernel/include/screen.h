#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BASE_CHAR_SIZE 8
extern int CHAR_WIDTH;
extern int CHAR_HEIGHT;
extern int font_scale;
extern int SCREEN_COLS;
extern int SCREEN_ROWS;
#define OFFSET_X 8
#define OFFSET_Y 8

#define BACKGROUND_COLOR 0x00

void screen_init(void);
void screen_clear(void);
void screen_put_char(int col, int row, char c, uint8_t color);
void screen_put_char_offset(int col, int row, char c, uint8_t color,
                            int off_x, int off_y);
void screen_update_metrics(void);
void screen_adjust_font_scale(int delta);

#endif

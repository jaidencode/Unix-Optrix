#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define SCREEN_COLS 38
#define SCREEN_ROWS 23

void screen_init(void);
void screen_clear(void);
void screen_put_char(int col, int row, char c, uint8_t color);

#endif

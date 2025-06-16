#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

typedef struct {
    int x, y, w, h;
    int visible;
} window_t;

void window_draw(window_t* win, uint8_t color);

#endif

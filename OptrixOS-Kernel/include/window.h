#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

typedef struct {
    int x, y, w, h;
    int visible;
    int state; /* 0=normal,1=max,2=min */
    const char *title;
} window_t;

void window_init(window_t *win, int x, int y, int w, int h, const char *title);
void window_draw(window_t* win, uint8_t color);
void window_handle_mouse(window_t *win, int mx, int my, int click);

#endif

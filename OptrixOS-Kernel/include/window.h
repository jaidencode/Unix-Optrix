#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

typedef struct {
    int x, y, w, h;
    int px, py, pw, ph; /* previous position for redraw */
    int visible;
    int state; /* 0=normal,1=max,2=min */
    uint8_t color;     /* window colour */
    uint8_t bg_color;  /* background colour behind window */
    const char *title;
} window_t;

void window_init(window_t *win, int x, int y, int w, int h,
                 const char *title, uint8_t color, uint8_t bg_color);
void window_draw(window_t* win);
void window_handle_mouse(window_t *win, int mx, int my, int click, int rclick);

#endif

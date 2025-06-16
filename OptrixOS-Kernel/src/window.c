#include "window.h"
#include "graphics.h"

void window_draw(window_t* win, uint8_t color) {
    if(!win || !win->visible) return;
    draw_rect(win->x, win->y, win->w, win->h, color);
}

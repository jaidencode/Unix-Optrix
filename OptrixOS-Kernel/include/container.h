#ifndef CONTAINER_H
#define CONTAINER_H

#include "window.h"

#define MAX_WINDOWS 8

void container_init(void);
window_t* container_create(int x, int y, int w, int h, const char *title,
                           uint8_t color, uint8_t bg_color);
void container_draw(void);
void container_handle_mouse(int mx, int my, int click);

#endif

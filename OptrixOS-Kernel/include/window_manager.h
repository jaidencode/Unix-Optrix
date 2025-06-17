#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "window.h"

void window_manager_init(void);
void window_manager_add(window_t *win);
void window_manager_draw(void);
void window_manager_handle_mouse(int mx, int my, int click);

#endif

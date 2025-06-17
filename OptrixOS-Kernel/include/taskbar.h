#ifndef TASKBAR_H
#define TASKBAR_H

#include "window.h"

void taskbar_init(void);
void taskbar_register(window_t *win);
void taskbar_unregister(window_t *win);
void taskbar_draw(void);
void taskbar_handle_click(int x, int y);

#endif

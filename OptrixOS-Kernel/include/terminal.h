#ifndef TERMINAL_H
#define TERMINAL_H
#include "window.h"
void terminal_init(void);
void terminal_run(window_t *win);
void terminal_set_window(window_t *win);
#endif

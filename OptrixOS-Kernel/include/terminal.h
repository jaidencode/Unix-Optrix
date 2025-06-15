#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdint.h>
void terminal_initialize(void);
void terminal_clear(void);
void terminal_write(const char* s);
void terminal_use_graphics(int on);
#endif // TERMINAL_H

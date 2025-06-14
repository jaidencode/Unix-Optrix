#ifndef VGA_H
#define VGA_H
#include <stdint.h>
void vga_clear(uint8_t color);
void vga_print_center(const char* s, uint8_t color);
void vga_print_char(int x, int y, char c, uint8_t color);
void vga_print_at(int x, int y, const char* s, uint8_t color);
#endif

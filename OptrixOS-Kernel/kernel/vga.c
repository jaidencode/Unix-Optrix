#include "vga.h"
#include <stdint.h>
#define VGA_PTR ((volatile uint8_t*)0xA0000)
void vga_clear(uint8_t color) {
    for (int i = 0; i < 320*200; ++i)
        VGA_PTR[i] = color;
}
void vga_print_center(const char* s, uint8_t color) {
    int len = 0; while (s[len]) ++len;
    int x = (320 - len*8)/2, y = 96;
    for (int i = 0; s[i]; ++i)
        vga_print_char(x+i*8, y, s[i], color);
}
void vga_print_char(int x, int y, char c, uint8_t color) {
    for (int row = 0; row < 8; ++row)
        for (int col = 0; col < 8; ++col)
            VGA_PTR[(y+row)*320 + x+col] = (c == ' ' ? color : color|((col^row)&1));
}
void vga_print_at(int x, int y, const char* s, uint8_t color) {
    for (int i=0; s[i]; ++i)
        vga_print_char(x+i*8, y, s[i], color);
}

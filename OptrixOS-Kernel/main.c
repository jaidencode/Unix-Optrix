#include <stdint.h>
#include "font8x8_basic.h"
#include "boot_params.h"

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void init_serial() {
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB
    outb(COM1 + 0, 0x03);    // 38400 baud
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static void serial_write(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(COM1, c);
}

static void log(const char* s) {
    while (*s) serial_write(*s++);
}

static void draw_char(int x, int y, char c, uint8_t color) {
    uint8_t* video = (uint8_t*)0xA0000;
    const uint8_t* glyph = font8x8_basic[(uint8_t)c];
    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                video[(y + row) * 320 + x + col] = color;
            }
        }
    }
}

static void draw_string_center(const char* s, uint8_t color) {
    int len = 0;
    for (const char* p = s; *p; ++p) len++;
    int x = (320 - len * 8) / 2;
    int y = (200 - 8) / 2;
    while (*s) {
        draw_char(x, y, *s++, color);
        x += 8;
    }
}

void main(void) {
    const char* msg = "OS Loaded";
    init_serial();
    log("Kernel start\n");

    // Clear graphics screen
    uint8_t* video = (uint8_t*)0xA0000;
    for (int i = 0; i < 320 * 200; i++) video[i] = 0x00;

    draw_string_center(msg, 0x0F);
}

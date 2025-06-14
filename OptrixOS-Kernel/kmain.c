#include <stdint.h>

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

void kmain(void) {
    const char *msg = "OptrixOS Kernel Loaded\n";
    volatile char *video = (volatile char*)0xb8000;

    init_serial();
    log(msg);

    for (int i = 0; msg[i]; i++) {
        video[i*2] = msg[i];
        video[i*2+1] = 0x07;
    }

    for(;;);
}

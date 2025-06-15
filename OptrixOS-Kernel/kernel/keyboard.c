#include "keyboard.h"
#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static int keyboard_present = 0;

static int wait_send(void) {
    for (int i=0;i<100000;i++) if(!(inb(0x64)&0x02)) return 1; return 0;
}
static int wait_recv(void) {
    for (int i=0;i<100000;i++) if(inb(0x64)&0x01) return 1; return 0;
}

int keyboard_detect(void) {
    if(!wait_send()) return 0;
    outb(0x60, 0xEE); // echo command
    if(!wait_recv()) return 0;
    if(inb(0x60) != 0xFA) return 0; // ack
    if(!wait_recv()) return 0;
    if(inb(0x60) != 0xEE) return 0; // echo reply
    return 1;
}

void keyboard_init(void) {
    keyboard_present = keyboard_detect();
}

static const char scancode_map[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=',8, 9,
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'', '`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',0,
};

char keyboard_getchar(void) {
    if(!keyboard_present) return 0;
    uint8_t sc;
    do {
        if(!wait_recv()) return 0;
        sc = inb(0x60);
    } while (sc & 0x80);
    if (sc < sizeof(scancode_map))
        return scancode_map[sc];
    return 0;
}

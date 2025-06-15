#include "keyboard.h"
#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void keyboard_init(void) {
    (void)inb; // nothing needed
}

static const char scancode_map[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=',8, 9,
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'', '`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',0,
};

char keyboard_getchar(void) {
    uint8_t sc;
    do {
        sc = inb(0x60);
    } while (sc & 0x80);
    if (sc < sizeof(scancode_map))
        return scancode_map[sc];
    return 0;
}

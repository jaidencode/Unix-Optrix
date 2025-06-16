#include "keyboard.h"
#include "ports.h"
#include <stdint.h>

static const char sc_ascii[] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=',
    '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,
    '\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',0,
};

static const char sc_shift[] = {
    0,  27, '!','@','#','$','%','^','&','*','(',' )','_','+',
    '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',0,
    '|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' ',0,
};

static int shift_pressed = 0;

char keyboard_getchar(void) {
    if(!(inb(0x64) & 1))
        return 0;
    uint8_t sc = inb(0x60);

    if(sc == 0x2A || sc == 0x36) { /* shift pressed */
        shift_pressed = 1;
        return 0;
    }
    if(sc == 0xAA || sc == 0xB6) { /* shift released */
        shift_pressed = 0;
        return 0;
    }

    if(sc & 0x80)
        return 0;
    if(sc >= sizeof(sc_ascii))
        return 0;

    return shift_pressed ? sc_shift[sc] : sc_ascii[sc];
}

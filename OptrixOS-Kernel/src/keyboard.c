#include "keyboard.h"
#include "ports.h"
#include <stdint.h>

#define KBUF_SIZE 32

static char kbuf[KBUF_SIZE];
static int khead = 0;
static int ktail = 0;

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

static void enqueue(char c) {
    int next = (khead + 1) % KBUF_SIZE;
    if(next != ktail) { /* drop char if buffer full */
        kbuf[khead] = c;
        khead = next;
    }
}

void keyboard_update(void) {
    while(inb(0x64) & 1) {
        uint8_t status = inb(0x64);
        if(status & 0x20) {
            /* mouse data present - do not consume */
            break;
        }

        uint8_t sc = inb(0x60);

        if(sc == 0x2A || sc == 0x36) { /* shift pressed */
            shift_pressed = 1;
            continue;
        }
        if(sc == 0xAA || sc == 0xB6) { /* shift released */
            shift_pressed = 0;
            continue;
        }

        if(sc & 0x80)
            continue; /* key released */
        if(sc >= sizeof(sc_ascii))
            continue;

        char c = shift_pressed ? sc_shift[sc] : sc_ascii[sc];
        if(c)
            enqueue(c);
    }
}

char keyboard_getchar(void) {
    keyboard_update();
    if(khead == ktail)
        return 0;
    char c = kbuf[ktail];
    ktail = (ktail + 1) % KBUF_SIZE;
    return c;
}

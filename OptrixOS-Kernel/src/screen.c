#include "screen.h"
#include "ports.h"
#include <stdint.h>

#define WIDTH 100
#define HEIGHT 30

static volatile uint16_t* const VIDEO = (uint16_t*)0xB8000;

void screen_init(void) {
    const char tl = '\xC9';
    const char tr = '\xBB';
    const char bl = '\xC8';
    const char br = '\xBC';
    const char hline = '\xCD';
    const char vline = '\xBA';

    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++) {
            uint8_t color = 0x00;             /* black background */
            char ch = ' ';
            if(y == 0 && x == 0) { ch = tl; color = 0x0F; }
            else if(y == 0 && x == WIDTH-1) { ch = tr; color = 0x0F; }
            else if(y == HEIGHT-1 && x == 0) { ch = bl; color = 0x0F; }
            else if(y == HEIGHT-1 && x == WIDTH-1) { ch = br; color = 0x0F; }
            else if(y == 0 || y == HEIGHT-1) { ch = hline; color = 0x0F; }
            else if(x == 0 || x == WIDTH-1) { ch = vline; color = 0x0F; }
            VIDEO[y*WIDTH + x] = ((uint16_t)color << 8) | (unsigned char)ch;
        }
    }

    const char *title = "OptrixOS Terminal";
    int len = 0;
    while(title[len]) len++;
    int start = (WIDTH - len) / 2;
    for(int i = 0; title[i]; i++)
        VIDEO[start + i] = ((uint16_t)0x0F << 8) | title[i];

    /* hide hardware cursor */
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

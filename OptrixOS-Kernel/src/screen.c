#include "screen.h"
#include "ports.h"
#include <stdint.h>

#define WIDTH 80
#define HEIGHT 25

static volatile uint16_t* const VIDEO = (uint16_t*)0xB8000;

void screen_init(void) {
    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++) {
            uint8_t color = 0x1;              /* blue background */
            char ch = ' ';
            if(y == 0 || y == HEIGHT-1) {
                color = 0x0F;                 /* border */
                ch = '=';
            } else if(x == 0 || x == WIDTH-1) {
                color = 0x0F;                 /* border */
                ch = '|';
            }
            VIDEO[y*WIDTH + x] = ((uint16_t)color << 8) | ch;
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

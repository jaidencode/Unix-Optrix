#include "screen.h"
#include <stdint.h>

#define WIDTH 80
#define HEIGHT 25

static volatile uint16_t* const VIDEO = (uint16_t*)0xB8000;

void screen_init(void) {
    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++) {
            uint8_t color = 0x1;              /* blue background */
            if(y == 0 || y == HEIGHT-1 || x == 0 || x == WIDTH-1)
                color = 0x0F;                 /* white border */
            VIDEO[y*WIDTH + x] = ((uint16_t)color << 8) | ' ';
        }
    }
}

#include "screen.h"
#include <stdint.h>

void boot_logo(void) {
    const char frames[] = "|/-\\";
    for(int t=0; t<20; t++) {
        for(int f=0; f<4; f++) {
            screen_put_char(SCREEN_COLS/2, SCREEN_ROWS/2, frames[f], 0x0E);
            for(volatile int d=0; d<1000000; d++); /* crude delay */
        }
    }
    screen_clear();
    screen_init();
}

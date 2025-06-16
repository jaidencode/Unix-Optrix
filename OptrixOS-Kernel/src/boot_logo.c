#include "screen.h"
#include <stdint.h>

void boot_logo(void) {
    const char *msg = "OptrixOS booting...";
    int msg_len = 18; /* length of message */
    int start_col = (SCREEN_COLS - msg_len) / 2;
    for(int i=0; i<msg_len; i++)
        screen_put_char(start_col + i, SCREEN_ROWS/2 - 1, msg[i], 0x0A);

    const char frames[] = "|/-\\";
    for(int t=0; t<20; t++) {
        for(int f=0; f<4; f++) {
            screen_put_char(SCREEN_COLS/2, SCREEN_ROWS/2 + 1, frames[f], 0x0E);
            for(volatile int d=0; d<1000000; d++); /* crude delay */
        }
    }
    screen_clear();
    screen_init();
}

#include "screen.h"
#include "terminal.h"
#include "window.h"
#include "driver.h"
#include "mem.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (64*1024)

void kernel_main(void) {
    screen_init();
    mem_init(HEAP_BASE, HEAP_SIZE);
    driver_init_all();

    static window_t term_win;
    window_init(&term_win, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                "Terminal", 0x07, 0x00);
    terminal_set_window(&term_win);
    terminal_init();
    terminal_run(&term_win);
}

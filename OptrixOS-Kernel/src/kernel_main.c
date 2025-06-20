#include "screen.h"
#include "terminal.h"
#include "driver.h"
#include "mem.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (1024*1024)

void kernel_main(void) {
    screen_init();
    mem_init(HEAP_BASE, HEAP_SIZE);
    driver_init_all();

    terminal_init();
    terminal_run();
}

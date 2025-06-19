#include "screen.h"
#include "terminal.h"
#include "driver.h"
#include "mem.h"
#include "fs.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (64*1024)

void kernel_main(void) {
    screen_init();
    mem_init(HEAP_BASE, HEAP_SIZE);
    driver_init_all();

    fs_load_sector_file(1, 1, "disksector.bin");

    terminal_init();
    terminal_run();
}

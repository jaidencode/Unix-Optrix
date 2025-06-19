#include "screen.h"
#include "terminal.h"
#include "driver.h"
#include "mem.h"
#include "disk.h"
#include "fs.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (64*1024)

void kernel_main(void) {
    screen_init();
    mem_init(HEAP_BASE, HEAP_SIZE);
    disk_init();
    fs_load_from_disk();
    driver_init_all();

    terminal_init();
    terminal_run();
}

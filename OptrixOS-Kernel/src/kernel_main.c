#include "screen.h"
#include "terminal.h"
#include "driver.h"
#include "mem.h"
#include "disk.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (64*1024)

void kernel_main(void) {
    screen_init();
    mem_init(HEAP_BASE, HEAP_SIZE);
    driver_init_all();

    /* create a simple in-memory disk and partition */
    disk_t* d = disk_create(512*1024, 512);
    (void)partition_create(d, 0, d->size / d->sector_size);

    terminal_init();
    terminal_run();
}

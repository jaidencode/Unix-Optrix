#include "screen.h"
#include "terminal.h"
#include "driver.h"
#include "mem.h"
#include "disk.h"
#include "mbr.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (64*1024)

void kernel_main(void) {
    screen_init();
    mem_init(HEAP_BASE, HEAP_SIZE);
    driver_init_all();

    /* create a simple in-memory disk and parse its MBR */
    disk_t* d = disk_create(512*1024, 512);
    uint8_t buf[512];
    disk_read(d, 0, buf, 1);
    mbr_part_entry parts[4];
    if(mbr_parse(buf, parts))
        partition_create(d, parts[0].start_lba, parts[0].sectors);

    terminal_init();
    terminal_run();
}

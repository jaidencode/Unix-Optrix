#include "screen.h"
#include "terminal.h"
#include "driver.h"
#include "mem.h"
#include "disk.h"
#include "mbr.h"
#include "vfs.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (64*1024)

void kernel_main(void) {
    screen_init();
    mem_init(HEAP_BASE, HEAP_SIZE);
    driver_init_all();

    /* create a simple in-memory disk */
    disk_t* d = disk_create(512*1024, 512);
    partition_t* part = NULL;

    /* try to parse an MBR from the first sector */
    uint8_t buf[512];
    mbr_part_entry parts[4];
    if(disk_read(d, 0, buf, 1) && mbr_parse(buf, parts)) {
        part = partition_create(d, parts[0].start_lba, parts[0].sectors);
    } else {
        /* fallback to a single partition covering the disk */
        part = partition_create(d, 0, d->size / d->sector_size);
    }
    (void)part; /* currently unused */

    terminal_init();
    terminal_run();
}

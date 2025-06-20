#include "screen.h"
#include "terminal.h"
#include "driver.h"
#include "mem.h"
#include "partition.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (64*1024)

void kernel_main(void) {
    screen_init();
    mem_init(HEAP_BASE, HEAP_SIZE);
    driver_init_all();

    terminal_init();

    partition_entry parts[2];
    partition_parse((void*)0x7c00, parts, 2);
    terminal_print("Partition1 LBA:");
    terminal_print_int(parts[0].start_lba);
    terminal_print("\n");
    terminal_print("Partition2 LBA:");
    terminal_print_int(parts[1].start_lba);
    terminal_print("\n");

    terminal_run();
}

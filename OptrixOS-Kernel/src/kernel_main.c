#include "screen.h"
#include "boot_logo.h"
#include "desktop.h"
#include "scheduler.h"
#include "driver.h"
#include "mem.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (64*1024)

static void desktop_task(void *arg) {
    (void)arg;
    desktop_init();
    desktop_run();
}

void kernel_main(void) {
    screen_init();
    boot_logo();
    mem_init(HEAP_BASE, HEAP_SIZE);
    scheduler_init();
    scheduler_add(desktop_task, 0);
    driver_init_all();
    scheduler_run();
}

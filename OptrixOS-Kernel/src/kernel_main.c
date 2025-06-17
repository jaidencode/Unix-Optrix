#include "screen.h"
#include "boot_logo.h"
#include "desktop.h"
#include "driver.h"
#include "mem.h"
#include "cpu_core.h"

/* simple heap placed at 0x200000 for illustration */
#define HEAP_BASE ((unsigned char*)0x200000)
#define HEAP_SIZE (64*1024)

void kernel_main(void) {
    screen_init();
    boot_logo();
    mem_init(HEAP_BASE, HEAP_SIZE);
    driver_init_all();
    desktop_init();
    cpu_core_init();
    cpu_core_add_task((task_func_t)desktop_run, NULL);
    cpu_core_run();
}

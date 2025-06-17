#include "cpu.h"
#include "driver.h"
#include "mem.h"

static cpu_info_t cpu_info;

static void init(void) {
    cpu_detect(&cpu_info);
    (void)cpu_info; /* info is stored for later use */
}

static driver_t drv = { "cpu", init, 0 };

void cpu_driver_register(void) {
    driver_register(&drv);
}

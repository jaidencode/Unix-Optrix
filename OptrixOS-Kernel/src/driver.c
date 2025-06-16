#include "driver.h"

#define MAX_DRIVERS 16

static driver_t *drivers[MAX_DRIVERS];
static int driver_count = 0;

void driver_register(driver_t *drv) {
    if(driver_count < MAX_DRIVERS)
        drivers[driver_count++] = drv;
}

void driver_init_all(void) {
    for(int i=0;i<driver_count;i++)
        if(drivers[i]->init)
            drivers[i]->init();
}

void driver_update_all(void) {
    for(int i=0;i<driver_count;i++)
        if(drivers[i]->update)
            drivers[i]->update();
}

#include "../OptrixOS-Kernel/include/driver.h"
#include "../hardware/timer.h"

void system_idle(void){
    driver_update_all();
    timer_tick();
}

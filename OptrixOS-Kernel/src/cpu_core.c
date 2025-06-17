#include "cpu_core.h"

void cpu_core_init(void) {
    scheduler_init();
}

void cpu_core_add_task(task_func_t func, void *arg) {
    scheduler_add(func, arg);
}

void cpu_core_run(void) {
    scheduler_run();
}

#ifndef CPU_CORE_H
#define CPU_CORE_H

#include "scheduler.h"

void cpu_core_init(void);
void cpu_core_add_task(task_func_t func, void *arg);
void cpu_core_run(void);

#endif

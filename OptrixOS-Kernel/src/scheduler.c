#include "scheduler.h"

#define MAX_TASKS 16

typedef struct {
    task_func_t func;
    void *arg;
} task_t;

static task_t tasks[MAX_TASKS];
static int task_count = 0;

void scheduler_init(void) {
    task_count = 0;
}

int scheduler_add(task_func_t func, void *arg) {
    if(task_count >= MAX_TASKS)
        return 0;
    tasks[task_count].func = func;
    tasks[task_count].arg = arg;
    task_count++;
    return 1;
}

void scheduler_run(void) {
    while(1) {
        for(int i = 0; i < task_count; i++) {
            if(tasks[i].func)
                tasks[i].func(tasks[i].arg);
        }
    }
}

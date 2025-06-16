#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef void (*task_func_t)(void *);

void scheduler_init(void);
int scheduler_add(task_func_t func, void *arg);
void scheduler_run(void);

#endif

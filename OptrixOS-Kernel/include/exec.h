#ifndef EXEC_H
#define EXEC_H

#include "window.h"

typedef void (*exec_func_t)(window_t*);

void exec_init(void);
void exec_register(const char* name, exec_func_t func);
int exec_run(const char* name);

#endif

#include "exec.h"
#include <stddef.h>

#define MAX_EXECS 10

typedef struct {
    const char* name;
    exec_func_t func;
} exec_entry;

static exec_entry table[MAX_EXECS];
static int exec_count = 0;

static int streq(const char* a, const char* b) {
    while(*a && *b) { if(*a!=*b) return 0; a++; b++; }
    return *a==*b;
}

void exec_init(void) {
    exec_count = 0;
}

void exec_register(const char* name, exec_func_t func) {
    if(exec_count < MAX_EXECS) {
        table[exec_count].name = name;
        table[exec_count].func = func;
        exec_count++;
    }
}

int exec_run(const char* name) {
    for(int i=0;i<exec_count;i++) {
        if(streq(table[i].name, name)) {
            window_t win;
            window_init(&win, 150, 150, 400, 250, name, 0x07, 0x17);
            window_draw(&win);
            table[i].func(&win);
            return 1;
        }
    }
    return 0;
}

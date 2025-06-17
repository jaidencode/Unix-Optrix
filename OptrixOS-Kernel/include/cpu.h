#ifndef CPU_H
#define CPU_H
#include <stdint.h>

typedef struct {
    char vendor[13];
    uint32_t features_ecx;
    uint32_t features_edx;
} cpu_info_t;

void cpu_detect(cpu_info_t *info);

#endif

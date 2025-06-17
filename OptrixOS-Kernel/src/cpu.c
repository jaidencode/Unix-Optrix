#include "cpu.h"

void cpu_detect(cpu_info_t *info) {
    if(!info) return;
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];
    __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    *(uint32_t*)&vendor[0] = ebx;
    *(uint32_t*)&vendor[4] = edx;
    *(uint32_t*)&vendor[8] = ecx;
    vendor[12] = 0;
    for(int i=0;i<13;i++) info->vendor[i]=vendor[i];
    __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    info->features_ecx = ecx;
    info->features_edx = edx;
}

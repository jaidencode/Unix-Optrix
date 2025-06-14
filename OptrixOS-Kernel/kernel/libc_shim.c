// kernel/libc_shim.c
#include <stddef.h>
#include <stdint.h>
void* memset(void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}
int abs(int x) { return x < 0 ? -x : x; }
int assert(int x) { while (!x) {} return 1; } // crude, disables asserts

// Dummy string functions
int strncmp(const char* a, const char* b, size_t n) {
    while (n-- && *a && *b && *a == *b) a++, b++;
    return n == (size_t)-1 ? 0 : (unsigned char)*a - (unsigned char)*b;
}
long strtol(const char* nptr, char** endptr, int base) {
    long val = 0; int sign = 1;
    while (*nptr == ' ' || *nptr == '\t') ++nptr;
    if (*nptr == '-') { sign = -1; ++nptr; }
    while (*nptr >= '0' && *nptr <= '9') { val = val * base + (*nptr - '0'); ++nptr; }
    if (endptr) *endptr = (char*)nptr;
    return val * sign;
}

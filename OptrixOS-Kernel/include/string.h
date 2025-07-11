#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

// Copy n bytes from src to dest
static inline void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

// Compare n bytes
static inline int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *pa = a, *pb = b;
    for (; n; --n, ++pa, ++pb)
        if (*pa != *pb) return *pa - *pb;
    return 0;
}

// Return length of string
static inline size_t strlen(const char *s) {
    size_t len = 0;
    while (*s++) ++len;
    return len;
}

// Compare strings
static inline int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { ++a; ++b; }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

// Copy string
static inline char *strcpy(char *dst, const char *src) {
    char *ret = dst;
    while ((*dst++ = *src++));
    return ret;
}

#endif // _STRING_H_

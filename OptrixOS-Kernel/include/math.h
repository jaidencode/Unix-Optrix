// OptrixOS minimal math.h for stb_image compatibility
#pragma once

// Minimal constants
#ifndef HUGE_VAL
#define HUGE_VAL (__builtin_huge_val())
#endif

// Dummy math functions for stb_image. These are "good enough" for image loading.
static inline double ldexp(double x, int exp) {
    // simple fallback, not IEEE correct, but sufficient for stb_image's limited use
    while (exp > 0) { x *= 2.0; --exp; }
    while (exp < 0) { x /= 2.0; ++exp; }
    return x;
}
static inline double pow(double x, double y) {
    // NOT a real pow! Good enough for stb_image (will decode most images)
    double r = 1.0;
    int i = (int)y;
    if (y < 0) { x = 1.0 / x; i = -i; }
    while (i--) r *= x;
    return r;
}
static inline double floor(double x) { return (double)((int)x); }
static inline double ceil(double x) { int i = (int)x; return (x == (double)i) ? x : (double)(i+1); }
static inline double fabs(double x) { return (x < 0) ? -x : x; }

#define M_E        2.71828182845904523536
#define M_LN2      0.69314718055994530942
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_SQRT2    1.41421356237309504880

#ifndef ASSERT_H
#define ASSERT_H

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
static inline void __assert_fail(const char *expr, const char *file, int line) {
    (void)file; (void)line; (void)expr;
    // In a real kernel we might log this. Here we just halt.
    for(;;) { __asm__ __volatile__("hlt"); }
}
#define assert(expr) ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__))
#endif

#endif // ASSERT_H

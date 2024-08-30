#ifndef __LIBC_ASSERT_H
#define __LIBC_ASSERT_H 1

#ifdef NDEBUG
#define assert(expression) ((void)0)
#else
#define assert(expression) ((expression) ? (void)0 : __assert_fail(__FILE__, __LINE__, #expression))
extern void __assert_fail(const char*, int, const char*) __attribute__((noreturn));
#endif

#endif

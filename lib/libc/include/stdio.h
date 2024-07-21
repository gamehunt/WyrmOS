#ifndef __LIBC_STDIO_H
#define __LIBC_STDIO_H 1

#include <stdarg.h>
#include <stddef.h>

#define EOF -1

int putchar(int ch);
int puts(const char *s);

int printf(const char* format, ...);
int sprintf(char* buf, const char* format, ...);
int snprintf(char* buf, size_t size, const char* format, ...);

int vprintf(const char *format, va_list arg_ptr);
int vsprintf(char *buf, const char *format, va_list arg_ptr);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif

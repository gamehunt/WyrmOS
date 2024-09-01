#ifndef __LIBC_STDLIB_H
#define __LIBC_STDLIB_H 1

#include <stddef.h>

int  atoi(const char* str);
void abort(void) __attribute__((noreturn));

void* __attribute__ ((malloc)) malloc(size_t);
void* __attribute__ ((malloc)) realloc(void*, size_t);

void free(void*);

#endif

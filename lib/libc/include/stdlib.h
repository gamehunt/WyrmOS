#ifndef __LIBC_STDLIB_H
#define __LIBC_STDLIB_H 1

#include <stddef.h>
#include <sys/wyrm.h>

CHEADER_START

#if !defined(__LIBK) && !defined(__KERNEL)
int   atexit(void (*)(void));
char* getenv(const char*);
int   setenv(const char *name, const char *value, int overwrite);
int   unsetenv(const char *name);
int   putenv(char *string);
void __attribute__((noreturn)) exit(int exit_code);
extern char** environ;
#endif

int  abs(int n);
int  atoi(const char* str);
void abort(void) __attribute__((noreturn));

void* __attribute__ ((malloc)) calloc( size_t num, size_t size );
void* __attribute__ ((malloc)) malloc(size_t);
void* __attribute__ ((malloc)) realloc(void*, size_t);

void free(void*);

CHEADER_END

#endif

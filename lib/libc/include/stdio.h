#ifndef __LIBC_STDIO_H
#define __LIBC_STDIO_H 1

#include <stdarg.h>
#include <stddef.h>
#include <sys/wyrm.h>

#define EOF -1

#define SEEK_SET 0

CHEADER_START

#if !defined(__LIBK) && !defined(__KERNEL)
typedef struct {
    int pad;
} FILE;

int     feof(FILE *stream);
int     fflush(FILE *stream);
long    ftell(FILE *stream);
FILE*   fopen(const char*, const char*);
int     fclose(FILE *fp);
int     fseek(FILE *stream, long offset, int origin);
int     vfprintf(FILE*, const char*, va_list);
int     fprintf(FILE * stream, const char * format, ...);
size_t  fread(void*, size_t, size_t, FILE*);
size_t  fwrite(const void*, size_t, size_t, FILE*);
void    setbuf(FILE*, char*);

extern FILE* _stdout;
extern FILE* _stdin;
extern FILE* _stderr;

#define stdout _stdout
#define stdin  _stdin
#define stderr _stderr
#endif

int putchar(int ch);
int puts(const char *s);

int printf(const char* format, ...);
int sprintf(char* buf, const char* format, ...);
int snprintf(char* buf, size_t size, const char* format, ...);

int vprintf(const char *format, va_list arg_ptr);
int vsprintf(char *buf, const char *format, va_list arg_ptr);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

CHEADER_END

#endif

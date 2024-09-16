#ifndef __LIBC_STRING_H
#define __LIBC_STRING_H 1

#include <stddef.h>
#include <sys/wyrm.h>

CHEADER_START

void* memcpy(void *dest, const void *src, size_t n);
void* memset(void *s, int c, size_t n);
void* memmove(void *dest, const void *src, size_t n); 
int   memcmp(const void *s1, const void *s2, size_t n); 

char*  strcpy(char* destination, const char* source);
char*  strncpy(char *dst, const char *src, size_t len);
char*  strtok(char* string, const char* delim);
size_t strlen(const char *s);
int    strcmp(const char* s1, const char* s2);
char*  strcat(char* destination, const char* source);
char*  strncat(char *destination, const char *append, size_t n);
char*  strdup(const char *str);
char*  strchr(const char *str, int ch);
char*  strchrnul(const char *str, int ch);
size_t strcspn(const char * string1, const char * string2);
int    strncmp(const char * string1, const char * string2, size_t num);
long   strtol(const char *start, char **end, int radix);
char*  strstr(const char *strB, const char *strA);
char*  strnstr(const char* big, const char* little, size_t len);

CHEADER_END

#endif

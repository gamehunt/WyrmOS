#ifndef __LIBC_CTYPE_H
#define __LIBC_CTYPE_H 1

#include <sys/wyrm.h>

CHEADER_START

int isalnum(int c); 
int isalpha(int c);
int isblank(int c); 
int iscntrl(int c); 
int isdigit(int c);
int isgraph(int c); 
int islower(int c); 
int isprint(int c); 
int ispunct(int c);
int isspace(int c);
int isupper(int c); 
int isxdigit(int c);

int toupper(int c);   
int tolower(int c);   

CHEADER_END

#endif

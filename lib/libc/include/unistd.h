#ifndef __LIBC_UNISTD_H
#define __LIBC_UNISTD_H 1

#include <sys/types.h>
#include <sys/wyrm.h>

CHEADER_START

pid_t getpid();
pid_t fork();
int execv(const char*, char* const[]);
int execve(const char*, char* const[], char* const[]);
int execvp(const char*, char* const[]);

CHEADER_END

#endif

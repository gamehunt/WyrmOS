#ifndef __LIBC_UNISTD_H
#define __LIBC_UNISTD_H 1

#include <sys/types.h>
#include <sys/wyrm.h>
#include <stdio.h>

CHEADER_START

#ifndef __LIBK
pid_t getpid();
int fork();
int execv(const char*, char* const[]);
int execve(const char*, char* const[], char* const[]);
int execvp(const char*, char* const[]);
unsigned int sleep(unsigned int seconds);
int usleep(unsigned long usec);
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int close(int fd);
#endif

CHEADER_END

#endif

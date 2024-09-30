#ifndef __LIBC_SIGNAL_H
#define __LIBC_SIGNAL_H 1

#include <sys/types.h>
#include <sys/signal.h>

#ifndef __LIBK
int raise(int signal);
int kill(pid_t pid, int sig);
void (*signal (int signal, void (*sigfunc) (int func)))(int);
#endif

#endif

#ifndef __LIBC_SIGNALS_H
#define __LIBC_SIGNALS_H 1

#include <stdint.h>

#define SIGHUP      1  /* Hangup */
#define SIGINT      2  /* Interupt */
#define SIGQUIT     3  /* Quit */
#define SIGILL      4  /* Illegal instruction */
#define SIGTRAP     5  /* A breakpoint or trace instruction has been reached */
#define SIGABRT     6  /* Another process has requested that you abort */
#define SIGEMT      7  /* Emulation trap XXX */
#define SIGFPE      8  /* Floating-point arithmetic exception */
#define SIGKILL     9  /* You have been stabbed repeated with a large knife */
#define SIGBUS      10 /* Bus error (device error) */
#define SIGSEGV     11 /* Segmentation fault */
#define SIGSYS      12 /* Bad system call */
#define SIGPIPE     13 /* Attempted to read or write from a broken pipe */
#define SIGALRM     14 /* This is your wakeup call. */
#define SIGTERM     15 /* You have been Schwarzenegger'd */
#define SIGUSR1     16 /* User Defined Signal #1 */
#define SIGUSR2     17 /* User Defined Signal #2 */
#define SIGCHLD     18 /* Child status report */
#define NSIG 19

typedef uint64_t sigset_t;

#endif

#ifndef __K_SYSCALL_H
#define __K_SYSCALL_H 1

#include <stdint.h>

#define SYSCALL_INT    0x80

typedef int (*syscall_handler)(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f);

void k_cpu_setup_syscalls();

#endif

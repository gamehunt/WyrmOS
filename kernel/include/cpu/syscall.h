#ifndef __K_SYSCALL_H
#define __K_SYSCALL_H 1

#include <stdint.h>

#define SYS_OPEN  0
#define SYS_READ  1
#define SYS_WRITE 2
#define SYS_FORK  3
#define SYS_EXIT  4

int k_invoke_syscall(uint64_t n, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f);

#endif

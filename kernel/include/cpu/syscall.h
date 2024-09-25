#ifndef __K_SYSCALL_H
#define __K_SYSCALL_H 1

#include <stdint.h>

int k_invoke_syscall(uint64_t n, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f);

#endif

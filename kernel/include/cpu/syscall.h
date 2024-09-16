#ifndef __K_SYSCALL_H
#define __K_SYSCALL_H 1

#include <stdint.h>

#define SYSCALL_INT    0x80
#define SYSCALL_AMOUNT 256

#define DEFINE_SYSCALL(func, argn) \
	int __syscall_##func(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f) { \
		switch(argn) { \
			case 0: \
				return func(); \
			case 1: \
				return func(a); \
			case 2: \
				return func(a, b); \
			case 3: \
				return func(a, b, c); \
			case 4: \
				return func(a, b, c, d); \
			case 5: \
				return func(a, b, c, d, e); \
			case 6: \
				return func(a, b, c, d, e, f); \
			default: \
				k_warn("Invalid syscall invocation: %s(%d)", #func, argn); \
		} \
	} 

typedef int (*syscall_handler)(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f);

void k_cpu_setup_syscalls();
void k_cpu_setup_syscall(uint16_t num, syscall_handler h);

#endif

#ifndef __K_CPU_INTERRUPT_H
#define __K_CPU_INTERRUPT_H 1

#include <stdint.h>

typedef struct {
	uintptr_t r15, r14, r13, r12;
	uintptr_t r11, r10, r9, r8;
	uintptr_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

	uint64_t int_no, err_code;

	uintptr_t rip, cs, rflags, rsp, ss;
} registers;

typedef void(*interrupt_handler)(registers*);

void k_cpu_int_init();
void k_cpu_int_setup_handler(uint8_t interrupt, interrupt_handler handler);

#endif

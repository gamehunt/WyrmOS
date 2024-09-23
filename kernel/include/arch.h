#ifndef __K_ARCH_H
#define __K_ARCH_H 1

#include <stdint.h>

struct registers {
	uintptr_t r15, r14, r13, r12;
	uintptr_t r11, r10, r9, r8;
	uintptr_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
	uintptr_t int_no, err_code;
	uintptr_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

typedef struct registers regs;

typedef struct {
	uint64_t    rsp;
	uint64_t    rbp;
	uint64_t    rip;

	volatile union page* pml;
	void*       kernel_stack;
} context;

extern uint64_t arch_get_ticks();
extern __attribute__((noreturn))      void arch_load_ctx(volatile context* ctx);
extern __attribute__((returns_twice)) int  arch_save_ctx(volatile context* ctx);

struct core;
extern void arch_set_core_base(struct core* addr);

extern void arch_user_jmp(uintptr_t entry, uintptr_t stack);

extern void k_mem_set_kernel_stack(uintptr_t stack);
#define arch_set_kernel_stack(stack) k_mem_set_kernel_stack(stack)

int arch_init();

#endif

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

extern __attribute__((noreturn))      void arch_load_ctx(volatile context* ctx);
extern __attribute__((returns_twice)) int  arch_save_ctx(volatile context* ctx);

struct core;
extern void arch_set_core_base(struct core* addr);

extern void      arch_user_jmp(uintptr_t entry, uintptr_t stack);
extern void      arch_user_jmp_exec(int argc, const char** argv, char** envp, uintptr_t entry, uintptr_t stack);
extern int       arch_enter_signal(uintptr_t entry, int sig, regs* r);
extern int       arch_exit_signal(regs* r);
extern uintptr_t arch_get_stack();
extern void      arch_fork_ret();

#ifdef __X86_64__

extern void k_mem_set_kernel_stack(uintptr_t stack);
#define arch_set_kernel_stack(stack) k_mem_set_kernel_stack(stack)
extern uint64_t k_dev_read_tsc();
#define arch_get_ticks() k_dev_read_tsc()
extern uint64_t k_dev_get_cpu_speed();
#define arch_get_cpu_speed() k_dev_get_cpu_speed()
#define arch_pause() __builtin_ia32_pause()
#define SIG_RET_MAGIC 0x123

#endif

#define SUBSECONDS_PER_SECOND 1000000

void arch_stacktrace(regs* r);
void arch_prepare_panic();
void arch_dump(regs* r);

int  arch_init();

#endif

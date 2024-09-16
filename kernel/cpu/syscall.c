#include "cpu/interrupt.h"
#include "dev/log.h"
#include <cpu/syscall.h>
#include <stddef.h>

#define SYS_TEST 0

static const syscall_handler __syscall_table[] = {
};

static const size_t __syscall_amount = sizeof(__syscall_table) / sizeof(syscall_handler);

void __syscall_dispatcher(regs* r) {
	if(r->rax >= __syscall_amount || !__syscall_table[r->rax]) {
		k_warn("Invalid syscall: %d", r->rax);
		return;
	}	
	r->rax = __syscall_table[r->rax](r->rdi, r->rsi, r->rdx, r->rcx, r->r8, r->r9);
}

extern void __syscall_stub(regs*);
void k_cpu_setup_syscalls() {
	k_cpu_int_setup_idt_entry(SYSCALL_INT, (uint64_t) __syscall_stub, 0x08, 0xEE);
    k_cpu_int_setup_handler(SYSCALL_INT, __syscall_dispatcher);
}

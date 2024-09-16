#include "cpu/interrupt.h"
#include "dev/log.h"
#include <cpu/syscall.h>

static syscall_handler __syscall_table[SYSCALL_AMOUNT] = {0};

void __syscall_dispatcher(regs* r) {
	if(r->rax < 0 || r->rax >= SYSCALL_AMOUNT || !__syscall_table[r->rax]) {
		k_warn("Invalid syscall: %d", r->rax);
		return;
	}	
	r->rax = __syscall_table[r->rax](r->rdi, r->rsi, r->rdx, r->rcx, r->r8, r->r9);
}

void k_cpu_setup_syscalls() {
	k_cpu_int_setup_handler(SYSCALL_INT, __syscall_dispatcher);
}

void k_cpu_setup_syscall(uint16_t num, syscall_handler h) {
	if(num < 0 || num >= SYSCALL_AMOUNT) {
		k_warn("Tried to setup invalid syscall: %d", num);
		return;
	}	
	if(__syscall_table[num]) {
		k_warn("Overwriting syscall %d", num);
	}
	__syscall_table[num] = h;
}

#include "cpu/syscall.h"
#include "cpu/_syscall.h"
#include "cpu/interrupt.h"

void __syscall_dispatcher(regs* r) {
	r->rax = k_invoke_syscall(r->rax, r->rdi, r->rsi, r->rdx, r->rcx, r->r8, r->r9);
}

extern void __syscall_stub(regs*);
void k_cpu_setup_syscalls() {
	k_cpu_int_setup_idt_entry(SYSCALL_INT, (uint64_t) __syscall_stub, 0x08, 0xEE, 0x0);
    k_cpu_int_setup_handler(SYSCALL_INT, __syscall_dispatcher);
}

#include "panic.h"
#include "asm.h"
#include "dev/log.h"

#include <stdarg.h>
#include <stdio.h>

#define PANIC_WIDTH 20

extern const char* __log_prefixes[];

__attribute__((noreturn)) void panic(regs* r, const char* message, ...){
	cli();

	k_crit("Kernel panic!");
	k_print("%s", __log_prefixes[CRITICAL]);
	va_list ap;
	va_start(ap, message);
	vprintf(message, ap);
	k_print("\r\n");
	k_crit("Dump:");

	if(r) {
		k_crit("int_no = %ld\terr_code = %#lx", r->int_no, r->err_code);
		k_crit("rip = %#.16lx\trsp = %#.16lx\trbp = %#.16lx", r->rip, r->rsp, r->rbp);
		k_crit("rax = %#.16lx\trbx = %#.16lx\trcx = %#.16lx", r->rax, r->rbx, r->rcx);
		k_crit("rdx = %#.16lx\trdi = %#.16lx\trsi = %#.16lx", r->rdx, r->rdi, r->rsi);
		k_crit("r8  = %#.16lx\tr9  = %#.16lx\tr10 = %#.16lx", r->r8, r->r9, r->r10);
		k_crit("r11 = %#.16lx\tr12 = %#.16lx\tr13 = %#.16lx", r->r11, r->r12, r->r13);
		k_crit("r14 = %#.16lx\tr15 = %#.16lx", r->r14, r->r15);
	} else {
		k_crit("Not available.");
	}

	hcf();
}

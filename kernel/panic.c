#include "mem/mem.h"
#include "string.h"
#include <panic.h>
#include <asm.h>
#include <dev/log.h>
#include <stdlib.h>
#include <symbols.h>

#include <stdarg.h>
#include <stdio.h>

#define PANIC_WIDTH 20

extern const char* __log_prefixes[];


struct stackframe {
  struct stackframe* rbp;
  uintptr_t rip;
};


static void __stacktrace(uintptr_t stack) {
	struct stackframe* frame = (struct stackframe*) stack;
	while(frame && frame->rip) {
		symbol* nearest = k_find_nearest_symbol(frame->rip);
		const char* src = "unknown";
		uintptr_t offset = 0;
		if(nearest) {
			src = nearest->name;
			offset = frame->rip - nearest->address;
		} else if (frame->rip >= HEAP_START && frame->rip < HEAP_END){
			src = "heap";	
			offset = frame->rip - HEAP_START;
		} else if (frame->rip >= VIRTUAL_BASE) {
			src = "@internal";
		}
		k_crit("%#.16lx: <%s + %#lx>", frame->rip, src, offset);
		frame = frame->rbp;
	}
}

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

	k_crit("Stacktrace:");

	register uintptr_t bp asm ("bp");
	__stacktrace(r ? r->rbp : bp);

	hcf();
}

EXPORT(panic);
EXPORT_INTERNAL(__stacktrace);

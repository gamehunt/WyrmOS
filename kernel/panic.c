#include "mem/mem.h"
#include <panic.h>
#include <asm.h>
#include <dev/log.h>
#include <symbols.h>

#include <stdarg.h>
#include <stdio.h>

#define PANIC_WIDTH 20
#define MAX_STACKTRACE_DEPTH 16

extern const char* __log_prefixes[];


struct stackframe {
  struct stackframe* rbp;
  uintptr_t rip;
};

const char* __get_location_str(uintptr_t address, uintptr_t* offset) {
	symbol* nearest = k_find_nearest_symbol(address);
	const char* src = "unknown";
	*offset = 0;
	if(nearest) {
		src = nearest->name;
		*offset = address - nearest->address;
	} else if (address >= HEAP_START && address < HEAP_END){
		src = "heap";	
		*offset = address - HEAP_START;
	} else if (address >= VIRTUAL_BASE) {
		src = "@internal";
	}
	return src;	
}

static void __stacktrace(regs* r) {
	uintptr_t offset;
	uintptr_t address = r ? r->rip : (uintptr_t) __builtin_return_address(0);
	const char* src = __get_location_str(address, &offset);
	k_crit("%#.16lx: <%s + %#x>", address, src, offset);
	register uintptr_t bp asm ("bp");
	uintptr_t stack = r ? r->rbp : bp;
	struct stackframe* frame = (struct stackframe*) stack;
	int depth = 0;
	while((uintptr_t) frame > KERNEL_LOWEST_ADDRESS && frame->rip) {
		uintptr_t offset;
		const char* src = __get_location_str(frame->rip,  &offset);
		k_crit("%#.16lx: <%s + %#lx>", frame->rip, src, offset);
		frame = frame->rbp;
		depth++;
		if(depth > MAX_STACKTRACE_DEPTH) {
			k_crit("Stacktrace depth reached, further entries omitted.");
			break;
		}
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
	__stacktrace(r);
	hcf();
}

EXPORT(panic);

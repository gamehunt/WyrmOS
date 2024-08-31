#include "panic.h"
#include "asm.h"

#include <stdarg.h>
#include <stdio.h>

#define PANIC_WIDTH 20

__attribute__((noreturn)) void panic(regs* r, const char* message, ...){
	cli();

	printf("Kernel panic!\r\n");

	va_list ap;
	va_start(ap, message);
	vprintf(message, ap);

	printf("\r\nDump:\r\n");

	if(r) {
		printf("int_no = %#.16lx\terr_code = %#.16lx\r\n", r->int_no, r->err_code);
		printf("rip = %#.16lx\trsp = %#.16lx\trbp = %#.16lx\r\n", r->rip, r->rsp, r->rbp);
		printf("rax = %#.16lx\trbx = %#.16lx\trcx = %#.16lx\r\n", r->rax, r->rbx, r->rcx);
		printf("rdx = %#.16lx\trdi = %#.16lx\trsi = %#.16lx\r\n", r->rdx, r->rdi, r->rsi);
		printf("r8  = %#.16lx\tr9  = %#.16lx\tr10 = %#.16lx\r\n", r->r8, r->r9, r->r10);
		printf("r11 = %#.16lx\tr12 = %#.16lx\tr13 = %#.16lx\r\n", r->r11, r->r12, r->r13);
		printf("r14 = %#.16lx\tr15 = %#.16lx\r\n", r->r14, r->r15);
	} else {
		printf("Not available.\r\n");
	}

	hcf();
}

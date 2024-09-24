#include <arch.h>
#include "cpu/interrupt.h"
#include "dev/cmos.h"
#include "dev/log.h"
#include "mem/paging.h"
#include "mem/pmm.h"
#include "mem/gdt.h"
#include "panic.h"
#include "proc/process.h"
#include "proc/lapic.h"

int arch_init() {
    k_mem_gdt_init();
    k_process_set_core(&cores[0]);
	k_mem_paging_init();
	k_mem_pmm_init();
    k_dev_cmos_init();
	k_cpu_int_init();
    k_mem_paging_set_pml(NULL);
    return 0;
}

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

void arch_stacktrace(regs* r) {
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

void arch_dump(regs* r) {
	k_crit("int_no = %ld\terr_code = %#lx", r->int_no, r->err_code);
	k_crit("rip = %#.16lx\trsp = %#.16lx\trbp = %#.16lx", r->rip, r->rsp, r->rbp);
	k_crit("rax = %#.16lx\trbx = %#.16lx\trcx = %#.16lx", r->rax, r->rbx, r->rcx);
	k_crit("rdx = %#.16lx\trdi = %#.16lx\trsi = %#.16lx", r->rdx, r->rdi, r->rsi);
	k_crit("r8  = %#.16lx\tr9  = %#.16lx\tr10 = %#.16lx", r->r8, r->r9, r->r10);
	k_crit("r11 = %#.16lx\tr12 = %#.16lx\tr13 = %#.16lx", r->r11, r->r12, r->r13);
	k_crit("r14 = %#.16lx\tr15 = %#.16lx", r->r14, r->r15);
}

void arch_prepare_panic() {
	for (int i = 0; i < core_count; ++i) {
		if (i == current_core->id) continue;
		lapic_send_ipi(cores[i].lapic_id, 0x447D);
	}
}

#include <assert.h>
#include <cpu/interrupt.h>
#include "cpu/_syscall.h"
#include "cpu/pic.h"
#include "dev/log.h"
#include "panic.h"
#include "proc/process.h"

#define MAX_INTERRUPT 256

typedef struct {
   uint16_t offset_low;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset_mid;        // offset bits 16..31
   uint32_t offset_hi;        // offset bits 32..63
   uint32_t zero;            // reserved
} __attribute__((packed)) idt_entry;

struct idtptr {
	uint16_t limit;
	uint64_t offset;
} __attribute__((packed));

idt_entry idt[MAX_INTERRUPT] = {0};
struct idtptr idt_pointer;

extern void load_interrupt_table(struct idtptr* ptr);

// attrib = 0b1DP0GATE
// 64-bit Interrupt Gate - Kernel = 0x8E
// 64-bit Trap Gate      - Kernel = 0x8F
// 64-bit Interrupt Gate - User   = 0xEE
// 64-bit Trap Gate      - User   = 0xEF
void k_cpu_int_setup_idt_entry(uint8_t num, uint64_t entry, uint16_t code, uint8_t attrib, uint8_t ist) {
	idt[num].selector = code;
	idt[num].type_attributes = attrib;
	idt[num].offset_low = entry & 0xFFFF;
	idt[num].offset_mid = (entry >> 16) & 0xFFFF;
	idt[num].offset_hi  = (entry >> 32) & 0xFFFFFFFF;
	idt[num].ist  = ist & 7;
	idt[num].zero = 0;
}

#define define_isr(num) extern void isr##num(regs*); 
#define setup_isr(num, desc) k_cpu_int_setup_idt_entry(num, (uint64_t) isr##num, 0x08, 0x8E, 0x0); __isr_descs[num] = desc;
#define setup_df(num, desc) k_cpu_int_setup_idt_entry(num, (uint64_t) isr##num, 0x08, 0x8E, 0x1); __isr_descs[num] = desc;
#define isr_desc(num) __isr_descs[(num)]

define_isr(0);
define_isr(1);
define_isr(2);
define_isr(3);
define_isr(4);
define_isr(5);
define_isr(6);
define_isr(7);
define_isr(8);
define_isr(9);
define_isr(10);
define_isr(11);
define_isr(12);
define_isr(13);
define_isr(14);
define_isr(15);
define_isr(16);
define_isr(17);
define_isr(18);
define_isr(19);
define_isr(20);
define_isr(21);
define_isr(22);
define_isr(23);
define_isr(24);
define_isr(25);
define_isr(26);
define_isr(27);
define_isr(28);
define_isr(29);
define_isr(30);
define_isr(31);

//IRQ
define_isr(32); // 0 
define_isr(33);  
define_isr(34);  
define_isr(35);  
define_isr(36);  
define_isr(37);  
define_isr(38);  
define_isr(39);
define_isr(40); // 8
define_isr(41);  
define_isr(42);  
define_isr(43);  
define_isr(44);  
define_isr(45);  
define_isr(46);  
define_isr(47); // 16  
                
define_isr(123); // APIC tick
define_isr(125); // IPI NMI

interrupt_handler handlers[MAX_INTERRUPT] = {0};
const char* __isr_descs[MAX_INTERRUPT]    = {0};

void __dispatch_interrupt(regs* r) {
	if(r->int_no > MAX_INTERRUPT) {
		panic(r, "Invalid interrupt: %d", r->int_no);
	}

    if(r->int_no == 39) {
        goto end;
    }

	if(!handlers[r->int_no]) {
		if(IS_IRQ(r->int_no)) {
			IRQ_ACK(INT_TO_IRQ(r->int_no));
			return;
		}
        if(!current_core->current_process || r->cs == 0x08) {
		    panic(r, "Unhandled interrupt: %s (%d)", isr_desc(r->int_no), r->int_no);
        } else {
            k_debug("Exception: %s", isr_desc(r->int_no));
            k_process_send_signal(current_core->current_process->pid, SIGSEGV);
        }
    } else {
	    handlers[r->int_no](r);
    } 

end:
    if(current_core->current_process && r->cs != 0x08) {
        k_process_invoke_signals(r);
    }
}

void k_cpu_int_init() {
	idt_pointer.limit  = sizeof(idt);
	idt_pointer.offset = (uint64_t) &idt;

	setup_isr(0,  "Zero division");
	setup_isr(1,  "Debug");
	setup_isr(2,  "NMI");
	setup_isr(3,  "Breakpoint");
	setup_isr(4,  "Overflow");
	setup_isr(5,  "Bound range exceeded");
	setup_isr(6,  "Invalid opcode");
	setup_isr(7,  "Device not available");
	setup_df (8,  "Double fault");
	setup_isr(9,  "Coprocessor segment overrun");
	setup_isr(10, "Invalid TSS");
	setup_isr(11, "Segment not present");
	setup_isr(12, "Stack-Segment fault");
	setup_isr(13, "General protection fault");
	setup_isr(14, "Page Fault");
	setup_isr(15, "Reserved");
	setup_isr(16, "FP Exception");
	setup_isr(17, "Alignment check");
	setup_isr(18, "Machine check");
	setup_isr(19, "SIMD Exception");
	setup_isr(20, "Virtualization exception");
	setup_isr(21, "Control Protection exception");
	setup_isr(22, "Reserved");
	setup_isr(23, "Reserved");
	setup_isr(24, "Reserved");
	setup_isr(25, "Reserved");
	setup_isr(26, "Reserved");
	setup_isr(27, "Reserved");
	setup_isr(28, "Hypervisor injection exception");
	setup_isr(29, "VMM Communication exception");
	setup_isr(30, "Security exception");
	setup_isr(31, "Reserved");

	//IRQ
	setup_isr(32, "IRQ0"); 
	setup_isr(33, "IRQ1"); 
	setup_isr(34, "IRQ2"); 
	setup_isr(35, "IRQ3"); 
	setup_isr(36, "IRQ4"); 
	setup_isr(37, "IRQ5"); 
	setup_isr(38, "IRQ6"); 
	setup_isr(39, "IRQ7");
	setup_isr(40, "IRQ8"); 
	setup_isr(41, "IRQ10");
	setup_isr(42, "IRQ11");
	setup_isr(43, "IRQ12");
	setup_isr(44, "IRQ13");
	setup_isr(45, "IRQ14");
	setup_isr(46, "IRQ15");
	setup_isr(47, "IRQ16");

    setup_isr(123, "APIC tick");
    setup_isr(125, "NMI");

	k_cpu_setup_syscalls();
    k_cpu_int_flush_idt();
	k_cpu_pic_init();
}

void k_cpu_int_flush_idt() {
	load_interrupt_table(&idt_pointer);
}

void k_cpu_int_setup_handler(uint8_t interrupt, interrupt_handler handler) {
	handlers[interrupt] = handler;
}

EXPORT(k_cpu_int_setup_handler)

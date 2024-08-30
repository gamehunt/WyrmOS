#include <cpu/interrupt.h>
#include "asm.h"
#include "panic.h"

typedef struct {
   uint16_t offset_low;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset_mid;        // offset bits 16..31
   uint32_t offset_hi;        // offset bits 32..63
   uint32_t zero;            // reserved
} idt_entry;

struct idtptr {
	uint16_t limit;
	uint64_t offset;
} __attribute__((packed));

idt_entry idt[256];
struct idtptr idt_pointer;

extern void load_interrupt_table(struct idtptr* ptr);

// attrib = 0b1DP0GATE
// 64-bit Interrupt Gate - Kernel = 0x8E
// 64-bit Trap Gate      - Kernel = 0x8F
// 64-bit Interrupt Gate - User   = 0xEE
// 64-bit Trap Gate      - User   = 0xEF
void __setup_idt_entry(uint8_t num, uint64_t entry, uint16_t code, uint8_t attrib) {
	idt[num].selector = code;
	idt[num].type_attributes = attrib;
	idt[num].offset_low = entry & 0xFFFF;
	idt[num].offset_mid = (entry >> 16) & 0xFFFF;
	idt[num].offset_hi  = (entry >> 32);
	idt[num].ist  = 0;
	idt[num].zero = 0;
}

#define define_isr(num) extern void isr##num(registers*)
#define setup_isr(num) __setup_idt_entry(num, (uint64_t) isr##num, 0x08, 0x8E)

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
define_isr(39); // 7
define_isr(40);  
define_isr(41);  
define_isr(42);  
define_isr(43);  
define_isr(44);  
define_isr(45);  
define_isr(46);  
define_isr(47); // 15  

void __dispatch_interrupt(registers* r) {
	panic("interrupt");
}

void k_cpu_int_init() {
	idt_pointer.limit  = sizeof(idt) - 1;
	idt_pointer.offset = (uint64_t) &idt_pointer;

	setup_isr(0);
	setup_isr(1);
	setup_isr(2);
	setup_isr(3);
	setup_isr(4);
	setup_isr(5);
	setup_isr(6);
	setup_isr(7);
	setup_isr(8);
	setup_isr(9);
	setup_isr(10);
	setup_isr(11);
	setup_isr(12);
	setup_isr(13);
	setup_isr(14);
	setup_isr(15);
	setup_isr(16);
	setup_isr(17);
	setup_isr(18);
	setup_isr(19);
	setup_isr(20);
	setup_isr(21);
	setup_isr(22);
	setup_isr(23);
	setup_isr(24);
	setup_isr(25);
	setup_isr(26);
	setup_isr(27);
	setup_isr(28);
	setup_isr(29);
	setup_isr(30);
	setup_isr(31);

	//IRQ
	setup_isr(32);
	setup_isr(33);
	setup_isr(34);
	setup_isr(35);
	setup_isr(36);
	setup_isr(37);
	setup_isr(38);
	setup_isr(39);
	setup_isr(40);
	setup_isr(41);
	setup_isr(42);
	setup_isr(43);
	setup_isr(44);
	setup_isr(45);
	setup_isr(46);
	setup_isr(47);

	load_interrupt_table(&idt_pointer);

	sti();
}

void k_cpu_int_setup_handler(uint8_t interrupt, interrupt_handler handler) {

}

#ifndef __K_CPU_INTERRUPT_H
#define __K_CPU_INTERRUPT_H 1

#include <symbols.h>
#include <stdint.h>

#define IRQ_BASE  32
#define IRQ_TO_INT(n) ((n) + IRQ_BASE)
#define INT_TO_IRQ(n) ((n) - IRQ_BASE)
#define IS_IRQ(n) (n >= IRQ_BASE)

struct registers {
	uintptr_t r15, r14, r13, r12;
	uintptr_t r11, r10, r9, r8;
	uintptr_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
	uintptr_t int_no, err_code;
	uintptr_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

typedef struct registers regs;

typedef void(*interrupt_handler)(regs*);

INTERNAL void k_cpu_int_init();
void k_cpu_int_setup_handler(uint8_t interrupt, interrupt_handler handler);
void k_cpu_int_setup_idt_entry(uint8_t num, uint64_t entry, uint16_t code, uint8_t attrib);

#define k_cpu_int_setup_isr_handler(isr, handler) k_cpu_int_setup_handler(isr, handler)
#define k_cpu_int_setup_irq_handler(irq, handler) k_cpu_int_setup_handler(IRQ_TO_INT(irq), handler)

#endif

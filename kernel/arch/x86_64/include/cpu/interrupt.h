#ifndef __K_CPU_INTERRUPT_H
#define __K_CPU_INTERRUPT_H 1

#include <symbols.h>
#include <stdint.h>
#include <arch.h>

#define IRQ_BASE  32
#define IRQ_TO_INT(n) ((n) + IRQ_BASE)
#define INT_TO_IRQ(n) ((n) - IRQ_BASE)
#define MAX_IRQ 16
#define IS_IRQ(n) (n >= IRQ_BASE && n <= IRQ_BASE + MAX_IRQ)

typedef void(*interrupt_handler)(regs*);

INTERNAL void k_cpu_int_init();
INTERNAL void k_cpu_int_flush_idt();
void k_cpu_int_setup_handler(uint8_t interrupt, interrupt_handler handler);
void k_cpu_int_setup_idt_entry(uint8_t num, uint64_t entry, uint16_t code, uint8_t attrib, uint8_t ist);

#define k_cpu_int_setup_isr_handler(isr, handler) k_cpu_int_setup_handler(isr, handler)
#define k_cpu_int_setup_irq_handler(irq, handler) k_cpu_int_setup_handler(IRQ_TO_INT(irq), handler)

#endif

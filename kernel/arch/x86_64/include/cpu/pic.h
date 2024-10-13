#ifndef __K_CPU_PIC_H
#define __K_CPU_PIC_H 1

#include <stdint.h>
#include <symbols.h>

INTERNAL void k_cpu_pic_init();

void k_cpu_pic_irq_ack(uint8_t irq);
void k_cpu_pic_mask_irq(uint8_t irq); 
void k_cpu_pic_unmask_irq(uint8_t irq); 

// #define IRQ_ACK(irq) k_cpu_pic_irq_ack(irq) -- TODO determine when we use PIC and when IOAPIC
#include <proc/lapic.h>
#define IRQ_ACK(irq) lapic_eoi()

#endif

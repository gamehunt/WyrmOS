#ifndef __K_CPU_PIC_H
#define __K_CPU_PIC_H 1

#include <stdint.h>

void k_cpu_pic_init();
void k_cpu_pic_irq_ack(uint8_t irq);

#define IRQ_ACK(irq) k_cpu_pic_irq_ack(irq)

#endif
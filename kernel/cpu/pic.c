#include <cpu/pic.h>
#include "asm.h"

#define PIC1           0x20
#define PIC1_COMMAND   PIC1
#define PIC1_OFFSET    0x20
#define PIC1_DATA      (PIC1+1)

#define PIC2           0xA0
#define PIC2_COMMAND   PIC2
#define PIC2_OFFSET    0x28
#define PIC2_DATA      (PIC2+1)

#define PIC_EOI        0x20

#define ICW1_ICW4      0x01
#define ICW1_INIT      0x10

#define PIC_WAIT() \
	do { \
		/* May be fragile */ \
		asm volatile("jmp 1f\n\t" \
		             "1:\n\t" \
		             "    jmp 2f\n\t" \
		             "2:"); \
	} while (0)

#define OUTB(port, data) outb(port, data); PIC_WAIT();

void k_cpu_pic_init() {
	/* Cascade initialization */
	OUTB(PIC1_COMMAND, ICW1_INIT|ICW1_ICW4); 
	OUTB(PIC2_COMMAND, ICW1_INIT|ICW1_ICW4); 

	/* Remap */
	OUTB(PIC1_DATA, PIC1_OFFSET); 
	OUTB(PIC2_DATA, PIC2_OFFSET); 

	/* Cascade identity with slave PIC at IRQ2 */
	OUTB(PIC1_DATA, 0x04); 
	OUTB(PIC2_DATA, 0x02); 

	/* Request 8086 mode on each PIC */
	OUTB(PIC1_DATA, 0x01); 
	OUTB(PIC2_DATA, 0x01); 

	OUTB(PIC1_DATA, 0xFB);
    OUTB(PIC2_DATA, 0xFF);
}

void k_cpu_pic_irq_ack(uint8_t irq) {
	if (irq >= 8) {
		outb(PIC2_COMMAND, PIC_EOI);
	}
	outb(PIC1_COMMAND, PIC_EOI);
}

void k_cpu_pic_mask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;
 
    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    OUTB(port, value);        
}
 
void k_cpu_pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;
 
    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    OUTB(port, value);        
}

EXPORT(k_cpu_pic_irq_ack)
EXPORT(k_cpu_pic_mask_irq)
EXPORT(k_cpu_pic_unmask_irq)

EXPORT_INTERNAL(k_cpu_pic_init)

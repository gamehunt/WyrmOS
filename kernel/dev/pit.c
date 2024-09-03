#include "cpu/pic.h"
#include <dev/pit.h>
#include <asm.h>

#define PIT_CH0_DATA 0x40
#define PIT_CH1_DATA 0x41
#define PIT_CH2_DATA 0x42
#define PIT_MODE     0x43
#define PIT_SCALE 1193180

void k_dev_pit_init() {
	outb(PIT_MODE, 0x36);  // Mode 3 at channel 0, lo/hi byte access
	uint32_t divisor = PIT_SCALE / 100;

    outb(PIT_CH0_DATA, divisor & 0xFF);  // Low byte
    outb(PIT_CH0_DATA, divisor >> 8);    // High byte

	k_cpu_pic_unmask_irq(0);
}

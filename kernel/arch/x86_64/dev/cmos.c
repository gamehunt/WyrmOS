#include "dev/log.h"
#include <dev/cmos.h>

#define DEFAULT_CPU_SPEED 3500

static uint64_t __cpu_speed = 0;

void k_dev_cmos_init() {
	uintptr_t end_lo, end_hi;
	uint32_t start_lo, start_hi;
	asm volatile (
		/* Disables and sets gating for channel 2 */
		"inb   $0x61, %%al\n"
		"andb  $0xDD, %%al\n"
		"orb   $0x01, %%al\n"
		"outb  %%al, $0x61\n"
		/* Configure channel 2 to one-shot, next two bytes are low/high */
		"movb  $0xB2, %%al\n" /* 0b10110010 */
		"outb  %%al, $0x43\n"
		/* 0x__9b */
		"movb  $0x9B, %%al\n"
		"outb  %%al, $0x42\n"
		"inb   $0x60, %%al\n"
		/*  0x2e__ */
		"movb  $0x2E, %%al\n"
		"outb  %%al, $0x42\n"
		/* Re-enable */
		"inb   $0x61, %%al\n"
		"andb  $0xDE, %%al\n"
		"outb  %%al, $0x61\n"
		/* Pulse high */
		"orb   $0x01, %%al\n"
		"outb  %%al, $0x61\n"
		/* Read TSC and store in vars */
		"rdtsc\n"
		"movl  %%eax, %2\n"
		"movl  %%edx, %3\n"
		/* In QEMU and VirtualBox, this seems to flip low.
		 * On real hardware and VMware it flips high. */
		"inb   $0x61, %%al\n"
		"andb  $0x20, %%al\n"
		"jz   2f\n"
		/* Loop until output goes low? */
	"1:\n"
		"inb   $0x61, %%al\n"
		"andb  $0x20, %%al\n"
		"jnz   1b\n"
		"rdtsc\n"
		"jmp   3f\n"
		/* Loop until output goes high */
	"2:\n"
		"inb   $0x61, %%al\n"
		"andb  $0x20, %%al\n"
		"jz   2b\n"
		"rdtsc\n"
	"3:\n"
		: "=a"(end_lo), "=d"(end_hi), "=r"(start_lo), "=r"(start_hi)
	);

    uintptr_t end   = ((end_hi & 0xFFFFffff)   << 32) | (end_lo & 0xFFFFffff);
	uintptr_t start = ((uintptr_t)(start_hi & 0xFFFFffff) << 32) | (start_lo & 0xFFFFffff);
	__cpu_speed = (end - start) / 10000;
    if(__cpu_speed == 0) {
        __cpu_speed = DEFAULT_CPU_SPEED;
    }
    k_debug("TSC speed: %ldMHZ", __cpu_speed);
}

uint64_t k_dev_get_cpu_speed() {
    return __cpu_speed;
}

#include "cpu/interrupt.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include <boot/limine.h>
#include <mem/mem.h>
#include <asm.h>
#include <debug.h>

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;


void _start(void) {
	DEBUG_INIT();

	printf("WyrmOS Kernel loading...\r\n");

    if (!LIMINE_BASE_REVISION_SUPPORTED) {
		printf("Unsupported LIMINE revision\r\n");
		goto end;
    }

	k_mem_init();
	k_cpu_int_init();

end:
    hcf();
}

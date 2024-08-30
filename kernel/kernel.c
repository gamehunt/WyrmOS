#include "mem/paging.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>
#include <string.h>

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

    if (framebuffer_request.response == NULL
    	    || framebuffer_request.response->framebuffer_count < 1) {
		printf("Framebuffer not found.\r\n");
	} else {
    	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    	for (size_t i = 0; i < framebuffer->height; i++) {
    	    volatile uint32_t *fb_ptr = framebuffer->address;
    	    fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xff0000;
    	}
	}

	k_mem_init();

	k_mem_paging_map(0x8040201000, 0);

end:
    hcf();
}

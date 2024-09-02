#include "dev/log.h"
#include "panic.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include <boot/limine.h>
#include <mem/mem.h>
#include <cpu/interrupt.h>
#include <fs/fs.h>
#include <asm.h>
#include <debug.h>

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0,
	.mode = LIMINE_PAGING_MODE_X86_64_4LVL
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;


void _start(void) {
	DEBUG_INIT();

	k_info("WyrmOS Kernel loading...\r\n");

    if (!LIMINE_BASE_REVISION_SUPPORTED) {
		k_error("Unsupported LIMINE revision\r\n");
		goto end;
    }

	k_mem_init();
	k_cpu_int_init();
	k_fs_init();

	k_dev_log_init();

	asm("int $0x1");

end:
    hcf();
}

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>
#include <string.h>

#include <boot/limine.h>
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
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

static const char* mmap_type2str(int type) {
	switch(type) {
		case LIMINE_MEMMAP_USABLE:
			return "Usable";
		case LIMINE_MEMMAP_RESERVED:
			return "RESERVED";
		case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
			return "ACPI RECLAIMABLE";
		case LIMINE_MEMMAP_ACPI_NVS:
			return "ACPI_NVS";
		case LIMINE_MEMMAP_BAD_MEMORY:
			return "BAD";
		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
			return "BOOTLOADER RECLAIMABLE";
		case LIMINE_MEMMAP_KERNEL_AND_MODULES:
			return "KERNEL";
		case LIMINE_MEMMAP_FRAMEBUFFER:
			return "FRAMEBUFFER";
		default:
			return "UNKNOWN";
	}
}

void _start(void) {
	DEBUG_INIT();
	DEBUG_PUTSTR("WyrmOS Kernel loading...\r\n");

    if (!LIMINE_BASE_REVISION_SUPPORTED) {
		DEBUG_PUTSTR("Unsupported LIMINE revision\r\n");
		goto end;
    }

	if(memmap_request.response == NULL 
			|| memmap_request.response->entry_count < 1) {
		DEBUG_PUTSTR("Memory map not found.\r\n");
	} else {
		DEBUG_PUTSTR("Memory map found.\r\n");
		for(uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
			struct limine_memmap_entry* entry = memmap_request.response->entries[i];
			printf("Mmap entry: %#.16lX - %#.16lX - %s\r\n", entry->base, entry->base + entry->length, mmap_type2str(entry->type));
		}
	}

    if (framebuffer_request.response == NULL
    	    || framebuffer_request.response->framebuffer_count < 1) {
		DEBUG_PUTSTR("Framebuffer not found.\r\n");
	} else {
    	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    	for (size_t i = 0; i < framebuffer->height; i++) {
    	    volatile uint32_t *fb_ptr = framebuffer->address;
    	    fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xff0000;
    	}
	}


end:
    hcf();
}

#include "exec/elf.h"
#include "exec/initrd.h"
#include "panic.h"
#include <stdlib.h>
#include <symbols.h>
#include <asm.h>
#include <boot/limine.h>
#include <cpu/interrupt.h>
#include <debug.h>
#include <dev/log.h>
#include <fs/fs.h>
#include <mem/mem.h>

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

__attribute__((used, section(".requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

void kernel_main(void) {
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
	k_setup_symbols();

	if(!module_request.response || !module_request.response->module_count) {
		k_error("Failed to load initrd.");
		goto end;
	}

	k_exec_initrd_init();

	struct limine_file** initrds = module_request.response->modules;
	for(size_t i = 0; i < module_request.response->module_count; i++) {
		k_info("Loading initrd: %s - %#.16lx - %#.16lx", initrds[i]->path, initrds[i]->address, initrds[i]->address + initrds[i]->size);
		k_exec_initrd_load(initrds[i]->address, initrds[i]->size);
	}

	k_fs_mount("/", "/dev/ram0", "initrd");

	fs_node* mod = k_fs_open("/initrd/modules/testmod.wrm");
	if(mod) {
		void* buffer = malloc(mod->size);
		k_fs_read(mod, 0, mod->size, buffer);
		struct module_info* module = k_elf_load_module(buffer);
		if(module) {
			k_info("%#.16lx", module);
			k_info("Module: %s", module->name);
			module->load();
			k_info("Loaded.");
		} else {
			k_error("Failed to load module.");
		}
	} else {
		k_error("Failed to find module.");
	}

end:
    hcf();
}

void _start(void) {
	kernel_main();
}

EXPORT_INTERNAL(kernel_main)

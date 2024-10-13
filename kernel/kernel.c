#include "dev/pci.h"
#include "dev/misc.h"
#include "dev/terminal.h"
#include "exec/exec.h"
#include "exec/initrd.h"
#include "exec/module.h"
#include "panic.h"
#include "proc/process.h"
#include <string.h>
#include <symbols.h>
#include <asm.h>
#include <globals.h>
#include <boot/limine.h>
#include <cpu/interrupt.h>
#include <debug.h>
#include <dev/log.h>
#include <fs/fs.h>
#include <mem/mem.h>

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

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

    if(arch_init()) {
        goto end;
    }

	k_fs_init();
	k_dev_log_init();
    k_dev_terminal_init();
    k_dev_pci_init();
	k_setup_symbols();

    k_dev_init_misc();

	if(!module_request.response || !module_request.response->module_count) {
		panic(NULL, "Failed to load initrd.");
	}

    k_exec_init();
	k_exec_initrd_init();

	struct limine_file** initrds = module_request.response->modules;
	for(size_t i = 0; i < module_request.response->module_count; i++) {
		k_info("Loading initrd: %s - %#.16lx - %#.16lx", initrds[i]->path, initrds[i]->address, initrds[i]->address + initrds[i]->size);
		k_exec_initrd_load(initrds[i]->address, initrds[i]->size);
	}

	k_fs_mount("/", "/dev/ram0", "initrd");

	k_process_init();
	k_load_modules();

    const char* argv[] = {"/bin/init", NULL};
	int r = k_exec("/bin/init", 1, argv, NULL);
	if(r != 0) {
		panic(NULL, "Failed to execute init process");
	}

	panic(NULL, "Init process died.");

end:
    hcf();
}

void _start(void) {
	__k_initial_stack = arch_get_stack();
	kernel_main();
}

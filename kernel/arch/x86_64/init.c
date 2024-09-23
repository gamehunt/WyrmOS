#include <arch.h>
#include "cpu/interrupt.h"
#include "mem/paging.h"
#include "mem/pmm.h"
#include "mem/gdt.h"
#include "proc/process.h"

int arch_init() {
    k_mem_gdt_init();
    k_process_set_core(&cores[0]);
	k_mem_paging_init();
	k_mem_pmm_init();
	k_cpu_int_init();
    return 0;
}

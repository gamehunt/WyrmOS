#include <proc/smp.h>
#include "cpu/interrupt.h"
#include "dev/acpi.h"
#include "dev/log.h"
#include "boot/limine.h"
#include "mem/mem.h"
#include "proc/process.h"

__attribute__((used, section(".requests")))
static volatile struct limine_smp_request cores_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

static void __init_core(struct limine_smp_info* info) {
    k_mem_flush_gdt();
    k_process_set_core(&cores[info->extra_argument]);
    k_cpu_int_flush_idt();
    k_process_init_core();
}

static int __try_bootloader() {
    if(!cores_request.response) {
        return 0;
    }

    struct limine_smp_response* response = cores_request.response;

    k_debug("Core count: %ld", response->cpu_count);

    for(uint64_t core = 0; core < response->cpu_count; core++) {
        if(response->cpus[core]->lapic_id == response->bsp_lapic_id) {
            continue;
        }
        response->cpus[core]->extra_argument = core;
        response->cpus[core]->goto_address   = __init_core;
    }

    return 1;
}

static int __try_acpi() {
    acpi_madt* madt = (acpi_madt*) k_dev_acpi_find_table("APIC"); 
    if(!madt) {
        return 0;
    }

    return 1;
}

static int __try_mp() {
    return 0;
}

void k_proc_init_cores() {
    if(__try_bootloader()) {
        return;
    }

    k_warn("Bootloader didn't provided APIC tables, will try ACPI.");

    if(__try_acpi()) {
        return;
    }

    k_warn("ACPI didn't provided APIC tables, will try MP.");

    if(__try_mp()) {
        return;
    }

    k_error("MP not found, giving up on SMP.");
}

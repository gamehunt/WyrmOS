#include <proc/smp.h>
#include "cpu/interrupt.h"
#include "dev/acpi.h"
#include "dev/log.h"
#include "boot/limine.h"
#include "dev/pit.h"
#include "mem/gdt.h"
#include "mem/mem.h"
#include "mem/mmio.h"
#include "mem/paging.h"
#include "proc/process.h"
#include "assert.h"

__attribute__((used, section(".requests")))
static volatile struct limine_smp_request cores_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

static uintptr_t __lapic_addr = 0;
static volatile int __ap_flag = 0;

static void __apic_tick(regs* r) {
    k_process_update_timings();
    if(r->cs != 0x08) {
        k_process_yield();
    }
}

static void __init_apic_timer() {
    assert(__lapic_addr >= MMIO_START);

    *(volatile uint32_t*) (__lapic_addr + 0x0F0) = 0x127;
    *(volatile uint32_t*) (__lapic_addr + 0x320) = 0x7B;
    *(volatile uint32_t*) (__lapic_addr + 0x3E0) = 0x01;

    /* Time our APIC timer against the TSC */
	uint64_t before = arch_get_ticks();
	*((volatile uint32_t*)(__lapic_addr + 0x380)) = 1000000;
    while (*((volatile uint32_t*)(__lapic_addr + 0x390))) {;}
	uint64_t after = arch_get_ticks();

	uint64_t ms = (after - before) / arch_get_cpu_speed();
	uint64_t target = 10000000000UL / ms;

	*((volatile uint32_t*)(__lapic_addr + 0x3e0)) = 1;
	*((volatile uint32_t*)(__lapic_addr + 0x320)) = 0x7B | 0x20000;
	*((volatile uint32_t*)(__lapic_addr + 0x380)) = target;
}

extern void __setup_flags();
static void __init_core(struct limine_smp_info* info) {
    __setup_flags();

    cores[info->extra_argument].id       = info->extra_argument;
    cores[info->extra_argument].lapic_id = info->lapic_id;
    cores[info->extra_argument].pml      = k_mem_paging_get_root_pml();

    k_mem_flush_gdt(info->extra_argument);
    k_process_set_core(&cores[info->extra_argument]);
    k_cpu_int_flush_idt();

	current_core->idle_process    = k_process_create_idle();
	current_core->current_process = current_core->idle_process;
    k_debug("core %d: ready", current_core->id);

    __ap_flag = 0;

    __init_apic_timer();
    k_process_schedule_next();
}

extern int core_count;
static int __try_bootloader() {
    if(!cores_request.response) {
        return 0;
    }

    k_cpu_int_setup_isr_handler(0x7B, __apic_tick);

    struct limine_smp_response* response = cores_request.response;

    core_count = response->cpu_count;
    if(core_count > MAX_CORES) {
        core_count = MAX_CORES;
    }

    k_debug("Core count: %ld", response->cpu_count);

    for(uint64_t core = 0; core < response->cpu_count; core++) {
        if(response->cpus[core]->lapic_id == response->bsp_lapic_id) {
            current_core->lapic_id = response->cpus[core]->lapic_id;
            __init_apic_timer();
            k_debug("core 0: ready");
            continue;
        }
        __ap_flag = 1;
        response->cpus[core]->extra_argument = core;
        response->cpus[core]->goto_address   = __init_core;
        do {
            arch_pause();
        } while(__ap_flag);
    }

    return 1;
}

static int __try_acpi(acpi_madt* madt) {
    return 0;
}

static int __try_mp() {
    return 0;
}

void k_proc_init_cores() {
    acpi_madt* madt = (acpi_madt*) k_dev_acpi_find_table("APIC"); 
    if(!madt) {
        k_error("Failed to locate MADT, giving up on SMP.");
        goto fallback;
    }

    __lapic_addr = (uintptr_t) k_mem_iomap(madt->lapic, PAGE_SIZE);
    k_debug("Mapped LAPIC to %#.16lx", __lapic_addr);

    if(__try_bootloader()) {
        return;
    }

    k_warn("Bootloader didn't provided APIC tables, will try ACPI.");

    if(__try_acpi(madt)) {
        return;
    }

    k_warn("ACPI didn't provided APIC tables, will try MP.");

    if(__try_mp()) {
        return;
    }

    k_error("MP not found, giving up on SMP.");

fallback:
    k_dev_pit_init();
}

void lapic_write(size_t addr, uint32_t value) {
	*((volatile uint32_t*)(__lapic_addr + addr)) = value;
	asm volatile ("":::"memory");
}

uint32_t lapic_read(size_t addr) {
	return *((volatile uint32_t*)(__lapic_addr + addr));
}

void lapic_send_ipi(int id, uint32_t ipi) {
	lapic_write(0x310, id << 24);
	lapic_write(0x300, ipi);
	do { asm volatile ("pause" : : : "memory"); } while (lapic_read(0x300) & (1 << 12));
}

void lapic_eoi() {
    lapic_write(0xB0, 0);
}

#include "dev/log.h"
#include "globals.h"
#include <mem/mem.h>
#include <mem/pmm.h>
#include <mem/paging.h>

#include <asm.h>
#include <util.h>

struct gdtptr {
	uint16_t size;
	uint64_t addr;
} __attribute__((packed));

union descriptor {
	struct {
		uint64_t limit: 16;
		uint64_t base : 24;
		uint64_t access: 8;
		uint64_t limit_upper: 4;
		uint64_t flags: 4;
		uint64_t base_upper: 8;
	} bits;
	uint64_t raw;
};

struct tss {
	uint32_t reserved0;
	uint64_t rsp[3];
	uint64_t ist[7];
	uint64_t reserved1;
	uint16_t reserved2;
	uint16_t iopb;
} __attribute__((packed));

struct extended_descriptor {
	union descriptor common;
	uint32_t         high;
	uint32_t         reserved;
};

union  descriptor gdt[7];
struct gdtptr     gdt_pointer;

union descriptor __encode_descriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	union descriptor result;
	result.bits.base        = base & 0x00FFFFFF;
	result.bits.base_upper  = base >> 24;
	result.bits.limit       = limit & 0xFFFF;
	result.bits.limit_upper = (limit >> 16) & 0xf;
	result.bits.access      = access;
	result.bits.flags       = flags & 0xf;
	return result;
}

volatile struct tss tss;

#define ACC_RW        (1 << 1)
#define ACC_DIRECTION (1 << 2)
#define ACC_EXEC      (1 << 3)
#define ACC_TYPE      (1 << 4)
#define ACC_USER      (3 << 5)
#define ACC_PRESENT   (1 << 7)

#define FL_LONG (1 << 1)
#define FL_SIZE (1 << 2)
#define FL_GRAN (1 << 3)

extern void load_descriptor_table(struct gdtptr* ptr);
extern void reload_segments();

static void __init_descriptors() {
	gdt_pointer.size = sizeof(gdt) - 1;
	gdt_pointer.addr = (uint64_t) &gdt;
	
	gdt[0] = __encode_descriptor(0, 0, 0, 0); // NULL 0x0
	gdt[1] = __encode_descriptor(0, 0xFFFFF, ACC_RW   | 
											 ACC_TYPE | 
											 ACC_EXEC | 
											 ACC_PRESENT, 
											 FL_LONG | 
											 FL_GRAN); // Kernel Code 0x8
	gdt[2] = __encode_descriptor(0, 0xFFFFF, ACC_RW   | 
											 ACC_TYPE | 
											 ACC_PRESENT, 
											 FL_LONG | 
											 FL_GRAN); // Kernel Data 0x10
	gdt[3] = __encode_descriptor(0, 0xFFFFF, ACC_RW   | 
											 ACC_TYPE | 
											 ACC_EXEC |
											 ACC_USER |
											 ACC_PRESENT, 
											 FL_LONG | 
											 FL_GRAN); // User Code 0x18
	gdt[4] = __encode_descriptor(0, 0xFFFFF, ACC_RW   | 
											 ACC_TYPE | 
											 ACC_USER |
											 ACC_PRESENT, 
											 FL_LONG | 
											 FL_GRAN); // User Data 0x20

	struct extended_descriptor* ext = (void*) &gdt[5]; // TSS 0x28
	ext->common = __encode_descriptor(((uintptr_t) &tss) & 0xFFFFFFFF, sizeof(tss), ACC_PRESENT | ACC_EXEC | 1, FL_SIZE);
	ext->high   = ((uintptr_t) &tss) >> 32;
	ext->reserved = 0;

	k_mem_set_kernel_stack(__k_initial_stack);
    k_mem_flush_gdt();
}

void k_mem_flush_gdt() {
	load_descriptor_table(&gdt_pointer);
	reload_segments();
}

void k_mem_set_kernel_stack(uintptr_t stack) {
	tss.rsp[0] = stack;
}

int k_mem_gdt_init() {
	__init_descriptors();
	return 0;
}

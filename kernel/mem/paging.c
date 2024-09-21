#include <mem/paging.h>
#include "mem/alloc.h"
#include "mem/pmm.h"
#include "cpu/interrupt.h"
#include "panic.h"
#include "proc/process.h"
#include "symbols.h"
#include <boot/limine.h>
#include <assert.h>
#include <string.h>

#define PME(addr) ((uint16_t) ((((uint64_t) addr & CANONICAL_MASK) >> 39) & 0x1FF))
#define PDP(addr) ((uint16_t) ((((uint64_t) addr & CANONICAL_MASK) >> 30) & 0x1FF))
#define PDE(addr) ((uint16_t) ((((uint64_t) addr & CANONICAL_MASK) >> 21) & 0x1FF))
#define PTE(addr) ((uint16_t) ((((uint64_t) addr & CANONICAL_MASK) >> 12) & 0x1FF))

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

addr __high_map_addr = 0;

static volatile union page* __root_pml = NULL;

extern union page* __get_pml(uint64_t map);
extern addr __get_pagefault_address();

static void __handle_pagefault(regs* r) {
	panic(r, "Page fault at %#.16lx.", __get_pagefault_address());
}

int k_mem_paging_init() {
	__high_map_addr   = hhdm_request.response->offset;
	current_core->pml =  __get_pml(__high_map_addr);
	__root_pml        = current_core->pml;
	k_cpu_int_setup_handler(14, __handle_pagefault);
	return 0;
}

volatile union page* k_mem_paging_get_current_pml() {
	return current_core->pml;
}

volatile union page* k_mem_paging_get_root_pml() {
	return __root_pml;
}

void k_mem_paging_map_ex(addr vaddr, addr paddr, uint8_t flags) {
	uint16_t pme = PME(vaddr);
	uint16_t pdp = PDP(vaddr);
	uint16_t pde = PDE(vaddr);
	uint16_t pte = PTE(vaddr);

	assert(pme <= 512 && pdp <= 512 && pde <= 512 && pte <= 512);

	uint16_t layers[4] = {pme, pdp, pde, pte};

	volatile union page* pg = current_core->pml;

	for(int i = 0; i < 4; i++) {
		uint16_t index = layers[i];
		if(!pg[index].bits.present) {
			union page p; 
			p.raw = 0;
			p.bits.present  = 1;
			p.bits.writable = 1;
			p.bits.user     = flags & PM_FL_USER;
			if(i < 3 || paddr == 0) {
				p.bits.page = FRAME(k_mem_pmm_alloc(1));
			} else {
				p.bits.page = FRAME(paddr);
			}
			pg[index] = p;
		}
		if(i != 3) {
			pg = (union page*) TO_VIRTUAL(ADDR(pg[index].bits.page));
		}
	}
}

void k_mem_paging_map_pages_ex(addr vaddr, size_t pages, addr paddr, uint8_t flags) {
	assert(vaddr % PAGE_SIZE == 0);
	assert(paddr % PAGE_SIZE == 0);
	for(size_t page = 0; page < pages; page++) {
		k_mem_paging_map_ex(vaddr + page * PAGE_SIZE, (paddr == 0 ? paddr : paddr + page * PAGE_SIZE), flags);
	}
}

void k_mem_paging_map_region_ex(addr vaddr_start, addr vaddr_end, addr paddr, uint8_t flags) {
	assert(vaddr_start % PAGE_SIZE == 0);
	assert(vaddr_end % PAGE_SIZE == 0);
	assert(paddr % PAGE_SIZE == 0);
	k_mem_paging_map_pages_ex(vaddr_start, (vaddr_start - vaddr_end) / PAGE_SIZE, paddr, flags);
}

void k_mem_paging_unmap(addr vaddr) {
	uint16_t pme = PME(vaddr);
	uint16_t pdp = PDP(vaddr);
	uint16_t pde = PDE(vaddr);
	uint16_t pte = PTE(vaddr);

	uint16_t layers[4] = {pme, pdp, pde, pte};

	volatile union page* pg = current_core->pml;

	for(int i = 0; i < 4; i++) {
		uint16_t index = layers[i];
		if(!pg[index].bits.present) {
			break;
		}
		if(i != 3) {
			pg = (union page*) TO_VIRTUAL(ADDR(pg[index].bits.page));
		} else {
			pg[index].raw = 0;
		}
	}
}

uintptr_t k_mem_paging_get_physical(addr vaddr) {
	uint64_t remainder = vaddr & (PAGE_SIZE - 1);
	vaddr &= ~(PAGE_SIZE - 1);

    assert(vaddr % PAGE_SIZE == 0);

	// if(vaddr & HIGH_MAP) {
	// 	return (vaddr & ~(HIGH_MAP)) + remainder;
	// }

	uint16_t pme = PME(vaddr);
	uint16_t pdp = PDP(vaddr);
	uint16_t pde = PDE(vaddr);
	uint16_t pte = PTE(vaddr);

	assert(pme <= 512 && pdp <= 512 && pde <= 512 && pte <= 512);

	uint16_t layers[4] = {pme, pdp, pde, pte};

	volatile union page* pg = current_core->pml;

	for(int i = 0; i < 4; i++) {
		uint16_t index = layers[i];
		if(!pg[index].bits.present) {
			return 0;
		}
		if(i < 3) {
			pg = (union page*) TO_VIRTUAL(ADDR(pg[index].bits.page));
		} else {
			return ADDR(pg[index].bits.page) + remainder;
		}
	}

	return 0;
}

volatile union page* k_mem_paging_clone_pml(volatile union page* pml) {
	volatile union page* copy = vmalloc(PAGE_SIZE);
	if(!pml) {
		pml = __root_pml;
	}
	memcpy((void*) copy, (const void*) pml, PAGE_SIZE);
	return copy;
}

extern void __set_pml(uintptr_t phys);

void k_mem_paging_set_pml(volatile union page* pml) {
	if(!pml) {
		pml = __root_pml;
	}
	current_core->pml = pml;
	uintptr_t phys = k_mem_paging_get_physical((addr) pml);
	assert(phys != 0);
	__set_pml(phys);
}

EXPORT(k_mem_paging_map_ex)
EXPORT(k_mem_paging_unmap)
EXPORT(k_mem_paging_map_pages_ex)
EXPORT(k_mem_paging_map_region_ex)
EXPORT(k_mem_paging_get_current_pml)
EXPORT(k_mem_paging_clone_pml)
EXPORT(k_mem_paging_set_pml)
EXPORT(k_mem_paging_get_physical)

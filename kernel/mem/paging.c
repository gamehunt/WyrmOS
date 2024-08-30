#include "mem/pmm.h"
#include <mem/paging.h>

#include <boot/limine.h>
#include <stdio.h>

#define PME(addr) ((uint16_t) (((uint64_t) addr)                  >> 39))
#define PDP(addr) ((uint16_t) ((((uint64_t) addr) & 0x7FFFFFFFFF) >> 30))
#define PDE(addr) ((uint16_t) ((((uint64_t) addr) & 0x3FFFFFFF)   >> 21))
#define PTE(addr) ((uint16_t) ((((uint64_t) addr) & 0x1FFFFF)     >> 12))

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

addr __high_map_addr = 0;

union page* __pml = 0;

extern union page* __get_pml(uint64_t map);

int k_mem_paging_init() {
	__high_map_addr = hhdm_request.response->offset;
	__pml           = __get_pml(__high_map_addr);
	return 0;
}

union page* k_mem_paging_get_current_pml() {
	return __pml;
}

void k_mem_paging_map(addr vaddr, addr paddr) {
	uint16_t pme = PME(vaddr);
	uint16_t pdp = PDP(vaddr);
	uint16_t pde = PDE(vaddr);
	uint16_t pte = PTE(vaddr);

	uint16_t layers[4] = {pme, pdp, pde, pte};

	union page* pg = __pml;

	for(int i = 0; i < 4; i++) {
		uint16_t index = layers[i];
		if(!pg[index].bits.present) {
			union page p; 
			p.bits.present  = 1;
			p.bits.writable = 1;
			p.bits.user     = 0;
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

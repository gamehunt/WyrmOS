#include <mem/paging.h>
#include "dev/log.h"
#include "mem/alloc.h"
#include "mem/mem.h"
#include "mem/pmm.h"
#include "cpu/interrupt.h"
#include "panic.h"
#include "proc/process.h"
#include "symbols.h"
#include "util.h"
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
    if(!current_core->current_process || r->cs == 0x08) {
	    panic(r, "Page fault at %#.16lx.", __get_pagefault_address());
    }
    k_process_exit(-1);
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

uintptr_t k_mem_paging_get_physical(addr _vaddr) {
    addr vaddr = (_vaddr & CANONICAL_MASK) & ~(PAGE_SIZE - 1);
    assert(vaddr % PAGE_SIZE == 0);

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
        if(pg[index].bits.size) {
            return ADDR(pg[index].bits.page) + (_vaddr & (MB(2) - 1));
        }
		if(i < 3) {
			pg = (union page*) TO_VIRTUAL(ADDR(pg[index].bits.page));
		} else {
			return ADDR(pg[index].bits.page) + (_vaddr & (PAGE_SIZE - 1));
		}
	}

	return 0;
}

volatile union page* k_mem_paging_clone_pml(volatile union page* pml) {
	volatile union page* copy = vmalloc(PAGE_SIZE);
	if(!pml) {
		pml = __root_pml;
	}
    for(uint16_t _pml = 0; _pml < 512; _pml++) {
        copy[_pml] = pml[_pml];
        if(!pml[_pml].bits.present || !pml[_pml].bits.user) {
            continue;
        } else {
            copy[_pml].bits.page = FRAME(k_mem_pmm_alloc(1));
        }
        volatile union page* __pml      = (volatile union page*) TO_VIRTUAL(ADDR(pml[_pml].bits.page));
        volatile union page* __pml_copy = (volatile union page*) TO_VIRTUAL(ADDR(copy[_pml].bits.page));
        for(uint16_t pd = 0; pd < 512; pd++) {
            __pml_copy[pd] = __pml[pd];
            if(!__pml[pd].bits.present || !__pml[pd].bits.user) {
                continue;
            } else {
                __pml_copy[pd].bits.page = FRAME(k_mem_pmm_alloc(1));
            }
            volatile union page* __pd = (volatile union page*) TO_VIRTUAL(ADDR(__pml[pd].bits.page));
            volatile union page* __pd_copy = (volatile union page*) TO_VIRTUAL(ADDR(__pml_copy[pd].bits.page));
            for(uint16_t pt = 0; pt < 512; pt++) {
                __pd_copy[pt] = __pd[pt];
                if(!__pd[pt].bits.present || !__pd[pt].bits.user) {
                    continue;
                } else {
                    __pd_copy[pt].bits.page = FRAME(k_mem_pmm_alloc(1));
                }
                volatile union page* __pt = (volatile union page*) TO_VIRTUAL(ADDR(__pd[pt].bits.page));
                volatile union page* __pt_copy = (volatile union page*) TO_VIRTUAL(ADDR(__pd_copy[pt].bits.page));
                for(uint16_t page = 0; page < 512; page++) {
                    __pt_copy[page] = __pt[page];
                    if(!__pt[page].bits.present || !__pt[page].bits.user) {
                        continue;
                    } else {
                        __pt_copy[page].bits.page = FRAME(k_mem_pmm_alloc(1));
                        memcpy((void*) TO_VIRTUAL(ADDR(__pt_copy[page].bits.page)), (void*) TO_VIRTUAL(ADDR(__pt[page].bits.page)), PAGE_SIZE);
                    }
                }
            }
        }
    }
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
    if(pml == __root_pml) {
        assert((phys | HIGH_MAP) == (uintptr_t) pml);
    }
	__set_pml(phys);
}

void k_mem_paging_free_pml(volatile union page* pml) {
    for(uint16_t _pml = 0; _pml < 512; _pml++) {
        if(!pml[_pml].bits.present || !pml[_pml].bits.user) {
            continue;
        }
        volatile union page* __pml = (volatile union page*) TO_VIRTUAL(ADDR(pml[_pml].bits.page));
        for(uint16_t pd = 0; pd < 512; pd++) {
            if(!__pml[pd].bits.present || !__pml[pd].bits.user) {
                continue;
            }  
            volatile union page* __pd = (volatile union page*) TO_VIRTUAL(ADDR(__pml[pd].bits.page));
            for(uint16_t pt = 0; pt < 512; pt++) {
                if(!__pd[pt].bits.present || !__pd[pt].bits.user) {
                    continue;
                }
                volatile union page* __pt = (volatile union page*) TO_VIRTUAL(ADDR(__pd[pt].bits.page));
                for(uint16_t page = 0; page < 512; page++) {
                    if(!__pt[page].bits.present || !__pt[page].bits.user) {
                        continue;
                    } else {
                        k_mem_pmm_mark_frame(__pt[page].bits.page);
                    }
                }
                k_mem_pmm_mark_frame(__pd[pt].bits.page);
            }
            k_mem_pmm_mark_frame(__pml[pd].bits.page);
        }
        k_mem_pmm_mark_frame(pml[_pml].bits.page);
    }
}
int k_mem_validate_pointer(void* ptr, size_t size) {
    uintptr_t start = (uintptr_t) ptr;
    uintptr_t end   = size ? (start + size) : start;

    for(uintptr_t addr = ADDR(FRAME(start)); addr <= ADDR(FRAME(end)); addr += PAGE_SIZE) {
        if(!k_mem_paging_get_physical(addr)) {
            return 0;
        }
    }

    return 1;
}

EXPORT(k_mem_paging_map_ex)
EXPORT(k_mem_paging_unmap)
EXPORT(k_mem_paging_map_pages_ex)
EXPORT(k_mem_paging_map_region_ex)
EXPORT(k_mem_paging_get_current_pml)
EXPORT(k_mem_paging_clone_pml)
EXPORT(k_mem_paging_set_pml)
EXPORT(k_mem_paging_get_physical)

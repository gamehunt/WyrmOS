#ifndef __K_MEM_PAGING_H
#define __K_MEM_PAGING_H 1

#include <stddef.h>
#include <stdint.h>

#include <mem/mem.h>

#define PAGE_SIZE 0x1000
#define _page_aligned __attribute__((aligned(PAGE_SIZE)))

#define HIGH_MAP __high_map_addr
#define TO_VIRTUAL(phys)  (phys | HIGH_MAP)
#define TO_PHYSICAL(virt) (virt & ~(HIGH_MAP)) // Works only for addresses, obtained with TO_VIRTUAL

union page {
    struct {
        uint64_t present:1;
        uint64_t writable:1;
        uint64_t user:1;
        uint64_t writethrough:1;
        uint64_t nocache:1;
        uint64_t accessed:1;
        uint64_t _available1:1;
        uint64_t size:1;
        uint64_t global:1;
        uint64_t _available2:3;
        uint64_t page:28;
        uint64_t reserved:12;
        uint64_t _available3:11;
        uint64_t nx:1;
    } bits;
    uint64_t raw;
};

extern addr __high_map_addr;

int k_mem_paging_init();

union page* k_mem_paging_get_current_pml();
union page* k_mem_paging_get_root_pml();
union page* k_mem_paging_clone_pml(union page* pml);
void        k_mem_paging_set_pml(union page* pml);

uintptr_t k_mem_paging_get_physical(addr vaddr);
void k_mem_paging_map(addr vaddr, addr paddr);
void k_mem_paging_map_pages(addr vaddr, size_t pages, addr paddr);
void k_mem_paging_map_region(addr vaddr_start, addr vaddr_end, addr paddr);
void k_mem_paging_unmap(addr vaddr);

#endif

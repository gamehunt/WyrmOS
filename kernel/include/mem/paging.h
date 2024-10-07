#ifndef __K_MEM_PAGING_H
#define __K_MEM_PAGING_H 1

#include <stddef.h>
#include <stdint.h>

#include <mem/mem.h>

#define _page_aligned __attribute__((aligned(PAGE_SIZE)))

#define HIGH_MAP __high_map_addr
#define TO_VIRTUAL(phys)  ((phys) | HIGH_MAP)
#define TO_PHYSICAL(virt) (k_mem_paging_get_physical(virt))

#define PM_FL_USER     (1 << 0)
#define PM_FL_NOCACHE  (1 << 1)
#define PM_FL_WRITABLE (1 << 2)

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

volatile union page* k_mem_paging_get_current_pml();
volatile union page* k_mem_paging_get_root_pml();
volatile union page* k_mem_paging_clone_pml(volatile union page* pml);
void k_mem_paging_free_pml(volatile union page* pml);
void k_mem_paging_set_pml(volatile union page* pml);

uintptr_t k_mem_paging_get_physical(addr vaddr);
void k_mem_paging_map_ex(addr vaddr, addr paddr, uint8_t flags);
void k_mem_paging_map_pages_ex(addr vaddr, size_t pages, addr paddr, uint8_t flags);
void k_mem_paging_map_region_ex(addr vaddr_start, addr vaddr_end, addr paddr, uint8_t flags);
#define k_mem_paging_map(vaddr, paddr) k_mem_paging_map_ex((vaddr), (paddr), 0);
#define k_mem_paging_map_pages(vaddr, pages, paddr) k_mem_paging_map_pages_ex((vaddr), (pages), (paddr), 0)
#define k_mem_paging_map_region(vaddr_start, vaddr_end, paddr) k_mem_paging_map_region_ex((vaddr_start), (vaddr_end), (paddr), 0)
void k_mem_paging_unmap(addr vaddr);
int  k_mem_validate_pointer(void* ptr, size_t size);
#define validate_ptr(ptr, size) k_mem_validate_pointer(ptr, size)

#endif

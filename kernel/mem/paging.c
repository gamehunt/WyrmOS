#include <mem/paging.h>

#include <boot/limine.h>

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

addr __high_map_addr = 0;

union page* __pml = 0;

extern union page* __get_pml();

int k_mem_paging_init() {
	__high_map_addr = hhdm_request.response->offset;
	__pml = __get_pml();
	return 0;
}

union page* k_mem_paging_get_current_pml() {
	return __pml;
}

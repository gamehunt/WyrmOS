#include <mem/mem.h>
#include <mem/pmm.h>
#include <mem/paging.h>

int k_mem_init() {
	int r = k_mem_pmm_init();
	if(r < 0) {
		return r;
	}
	return 0;
}

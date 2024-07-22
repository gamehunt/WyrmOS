#include <mem/mem.h>
#include <mem/pmm.h>
#include <mem/paging.h>

#include <util.h>

int k_mem_init() {
	int r;
	_R(k_mem_paging_init(), r)
	// _R(k_mem_pmm_init(), r)
	return 0;
}

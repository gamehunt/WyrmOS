#include <mem/alloc.h>
#include <mem/mem.h>
#include <mem/paging.h>
#include <panic.h>

#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>

#define MIN_SLAB 3
#define MAX_SLAB 10
#define BIG_SLAB MAX_SLAB + 1
#define SLAB_AMOUNT  (BIG_SLAB - MIN_SLAB + 1)
#define SLAB_SIZE(s) (1 << s)
#define MAX_SLAB_SIZE SLAB_SIZE(MAX_SLAB)
#define MIN_SLAB_SIZE SLAB_SIZE(MIN_SLAB)

struct slab {
	size_t  size;
	void*   free;
	struct slab* next;
};

static void*  heap = (void*) HEAP_START;
static struct slab* slabs[SLAB_AMOUNT] = { NULL };
static struct slab* free_slabs = NULL;


static void* __sbrk(size_t pages) {
	assert(pages > 0);

	if((uintptr_t) heap + pages * PAGE_SIZE > HEAP_END) {
		panic(NULL, "Out of memory. Tried to allocate %d pages", pages);
	}
	
	void* r = heap;
	k_mem_paging_map_pages((uintptr_t) heap, pages, 0);
	heap += pages * PAGE_SIZE;

	return r;
}

static uint8_t __slab_for_size(size_t size) {
	if(size > MAX_SLAB_SIZE) {
		return BIG_SLAB;
	}
	if(size <= MIN_SLAB_SIZE) {
		return MIN_SLAB;
	}
	uint8_t slab = sizeof(size) * CHAR_BIT - __builtin_clzl(size);
	slab -= !(size & (size - 1));
	return slab;
}

static uint8_t __slab_index(size_t slab) {
	assert(slab >= MIN_SLAB);
	if(slab >= BIG_SLAB) {
		slab = BIG_SLAB;
	}
	slab -= MIN_SLAB;
	assert(slab < SLAB_AMOUNT);
	return slab;
}

static struct slab* __tail(struct slab* head) {
	struct slab* r = head;
	while(r && r->next) {
		r = r->next;
	}
	return r;
}

static void __insert_slab(struct slab* slab) {
	uint8_t index = __slab_index(slab->size);
	if(!slabs[index]) {
		slabs[index] = slab;
	} else {
		__tail(slabs[index])->next = slab;
	}
}

static void __insert_free_slab(struct slab* slab) {
	if(!free_slabs) {
		free_slabs = slab;
	} else {
		struct slab* parent = free_slabs;
		struct slab* prev   = NULL;
		while(parent && parent > slab) {
			prev   = parent;
			parent = parent->next;
		}
		if(!parent) {
			prev->next = slab;
		} else {
			slab->next = parent->next;
			parent->next = slab;
		}
	}
}

static struct slab* __try_get_free_slab() {
	if(!free_slabs) {
		return NULL;
	}
	struct slab* r = free_slabs;
	free_slabs = free_slabs->next;
	return r;
}

static struct slab* __try_get_big_free_slab(size_t size) {
	if(!free_slabs) {
		return NULL;
	}

	struct slab* slab = free_slabs;

	struct slab* head = NULL;
	struct slab* tail = slab->next;

	size_t found_size = 0;

	while(slab) {
		struct slab* iterator = slab;
		if(!found_size) {
			found_size = PAGE_SIZE - sizeof(struct slab);
		} else {
			found_size += PAGE_SIZE;
		}
		if(found_size >= size) {
			tail = iterator->next;
			break;
		}
		if((uintptr_t) iterator + PAGE_SIZE == (uintptr_t) iterator->next) {
			iterator = iterator->next;
		} else {
			head = slab;
			slab = iterator->next;
			found_size = 0;
		}
	}

	if(found_size >= size) {
		if(!head) {
			free_slabs = tail;
		} else {
			head->next = tail;
		}
		return slab;
	}

	return NULL;
} 

static void __free_big_slab(struct slab* s) {
	size_t real_size = s->size + sizeof(struct slab);
	size_t slabs = (real_size) / PAGE_SIZE + !!(real_size % PAGE_SIZE);
	for(size_t slab = 0; slab < slabs; slab++) {
		s->size = 0;
		s->free = NULL;
		s->next = NULL;
		__insert_free_slab(s);	
		s = (void*) ((uintptr_t) s + PAGE_SIZE);
	}
}

static void __slab_push(struct slab* slab, void* addr) {
	uintptr_t** item = addr;
	*item = slab->free;
	slab->free = item;
}

static void* __slab_pop(struct slab* slab) {
	void* item = slab->free;
	if(slab->size > MAX_SLAB) {
		slab->free = NULL;
	} else {
		slab->free = *((uintptr_t**) item);
	}
	return item;
}

static struct slab* __allocate_slab(uint8_t size) {
	assert(size <= MAX_SLAB);
	struct slab* s = __try_get_free_slab();
	if(!s) {
		s = __sbrk(1);
	}
	s->size = size;
	s->free = (void*) (((uintptr_t) s) + sizeof(struct slab));
	size_t entries = ((PAGE_SIZE - sizeof(struct slab)) / SLAB_SIZE(size)) - 1;
	uintptr_t** stack = s->free;
	uint8_t index = __slab_index(size);
	for(size_t e = 0; e < entries; e++) {
		stack[e << index] = (uintptr_t*) &stack[(e + 1) << index];
	}
	stack[entries << index] = NULL;
	s->next = NULL;
	return s;
}

static struct slab* __allocate_big_slab(size_t size) {
	struct slab* s = __try_get_big_free_slab(size);
	if(!s) {
		size_t real_size = size + sizeof(struct slab);
		s = __sbrk(real_size / PAGE_SIZE + !!(real_size % PAGE_SIZE));
	}
	s->size = size;
	s->free = (void*) (((uintptr_t) s) + sizeof(struct slab));
	s->next = NULL;
	return s;
}

#define HAS_FREE_SPACE(slab) (slab->free != NULL)

struct slab* __get_slab(size_t bytes) {
	uint8_t size  = __slab_for_size(bytes);
	uint8_t index = __slab_index(size);
	struct slab* r = __tail(slabs[index]);
	if(!r || !HAS_FREE_SPACE(r)) {
		if(size == BIG_SLAB) {
			r = __allocate_big_slab(bytes);
		} else {
			r = __allocate_slab(size);
		}
		__insert_slab(r);
	} 	
	return r;
}

void* __attribute__ ((malloc))  kmalloc(size_t bytes) {
	assert(bytes > 0);
	struct slab* s = __get_slab(bytes);
	void* item = __slab_pop(s);
	return item;
}

void* __attribute__ ((malloc)) kmalloc_aligned(size_t bytes, size_t align) {
	uintptr_t true_size = bytes + align - sizeof(struct slab);
	void * result = kmalloc(true_size);
	void * out = (void *)((uintptr_t)result + (align) - sizeof(struct slab));
	assert((uintptr_t) out % align == 0);
	return out;
}

void kfree(void* mem) {
	assert((uintptr_t) mem >= HEAP_START && (uintptr_t) mem < HEAP_END);

	if((uintptr_t) mem % PAGE_SIZE == 0) {
		mem = (void*) ((uintptr_t) mem - 1);
	}

	struct slab* s = (struct slab*) ((uintptr_t)mem & ~0xFFF);
	
	if(s->size > MAX_SLAB) {
		__free_big_slab(s);
	} else {
		__slab_push(s, mem);
	}
}

EXPORT(kfree);
EXPORT(kmalloc_aligned);
EXPORT(kmalloc);

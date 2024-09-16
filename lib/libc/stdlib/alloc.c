#include <stdlib.h>
#include <string.h>

#ifdef __LIBK
#include <mem/alloc.h>
#endif

void* __attribute__ ((malloc)) malloc(size_t size) {
#ifdef __LIBK
	return kmalloc(size);
#else
	return NULL;
#endif
}

void* __attribute__ ((malloc)) realloc(void* old, size_t new) {
	void* r = malloc(new);
	memmove(r, old, new);
	free(old);
	return r;
}

void* __attribute__ ((malloc)) calloc(size_t num, size_t size) {
	void* m = malloc(num * size);
    memset(m, 0, num * size);
    return m;
}

void free(void* ptr) {
#ifdef __LIBK
	kfree(ptr);
#endif
}

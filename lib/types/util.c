#include <types/util.h>

static int8_t __default_cmp(void* v1, void* v2) {
    if (v1 > v2) {
        return 1;
    } else if(v1 < v2) {
        return -1;
    } else {
        return 0;
    }
}

const comparator DEFAULT_COMPARATOR = __default_cmp;

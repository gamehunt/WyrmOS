#ifndef __TYPES_UTIL_H
#define __TYPES_UTIL_H 1

#include <stdint.h>

typedef int8_t(*comparator)(void*, void*);

extern const comparator DEFAULT_COMPARATOR;

#endif

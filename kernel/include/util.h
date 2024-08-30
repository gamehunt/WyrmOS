#ifndef __K_UTIL_H
#define __K_UTIL_H 1

#define _R(x, r) if((r = x) < 0) { return r; } 

#define KB(x) ((uint64_t) (x) * 1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)
#define TB(x) (GB(x) * 1024)

#define ALIGN(value, alignment) \
	(value + (-value & (alignment - 1)))

#endif

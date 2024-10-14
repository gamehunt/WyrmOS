#ifndef __K_SPINLOCK_H
#define __K_SPINLOCK_H 1

#include <stdatomic.h>
#include <arch.h>
#include <panic.h>

typedef volatile struct {
    volatile atomic_flag lock;
#ifndef NO_LOCK_CHECKS
    uint64_t    time;
#endif
} lock;

#ifndef NO_LOCK_CHECKS
#define LOCK_INIT_CHECK(_lock) \
        _lock.time = arch_get_ticks()
#define LOCK_CHECK(_lock) \
        if(arch_get_ticks() - _lock.time > 10000000000000UL) { \
            panic(NULL, "Deadlock detected: %s", #_lock); \
        } 
#else
#define LOCK_INIT_CHECK(_lock)
#define LOCK_CHECK(_lock)
#endif

#define LOCK(_lock) \
    do { \
        LOCK_INIT_CHECK(_lock); \
        while(atomic_flag_test_and_set_explicit( &_lock.lock, memory_order_acquire)) { \
            LOCK_CHECK(_lock) \
            arch_pause(); \
        } \
    } while(0);

#define UNLOCK(_lock) \
    do { \
        atomic_flag_clear_explicit( &_lock.lock, memory_order_release ); \
    } while(0);

#define EMPTY_LOCK (volatile lock) {.lock = ATOMIC_FLAG_INIT}

#endif

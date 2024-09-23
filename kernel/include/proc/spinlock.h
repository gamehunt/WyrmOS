#ifndef __K_SPINLOCK_H
#define __K_SPINLOCK_H 1

#include "sys/types.h"
#include <stdatomic.h>
#include <arch.h>

typedef struct {
    pid_t       owner;
    atomic_flag lock;
} lock;

#define LOCK(_lock) \
    while(atomic_flag_test_and_set_explicit( &_lock.lock, memory_order_acquire)) { \
        arch_pause(); \
    }

#define UNLOCK(_lock) \
    atomic_flag_clear_explicit( &_lock.lock, memory_order_release );

#define EMPTY_LOCK {.lock = ATOMIC_FLAG_INIT}

#endif

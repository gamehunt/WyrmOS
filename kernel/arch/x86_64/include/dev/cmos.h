#ifndef __K_DEV_CMOS_H
#define __K_DEV_CMOS_H 1

#include <stdint.h>

void            k_dev_cmos_init();
extern uint64_t k_dev_read_tsc();
uint64_t k_dev_get_cpu_speed();

#endif

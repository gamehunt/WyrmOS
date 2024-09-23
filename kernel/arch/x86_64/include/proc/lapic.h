#ifndef __K_LAPIC_H
#define __K_LAPIC_H 1

#include <stdint.h>
#include <stddef.h>

void     lapic_send_ipi(int id, uint32_t ipi);
void     lapic_write(size_t addr, uint32_t value);
uint32_t lapic_read(size_t addr);
void     lapic_eoi();

#endif

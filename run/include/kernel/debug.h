#ifndef __K_DEBUG_H
#define __K_DEBUG_H 1

#include <dev/serial.h>

#define DEBUG_PUTCHAR(c) k_dev_serial_putchar(COM1, c)
#define DEBUG_PUTSTR(str) k_dev_serial_putstr(COM1, str)

#endif

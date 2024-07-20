#ifndef __K_DEV_SERIAL_H
#define __K_DEV_SERIAL_H 1

#include <stdint.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8
#define COM5 0x5F8
#define COM6 0x4F8
#define COM7 0x5E8
#define COM8 0x4E8

int     k_dev_serial_init(uint16_t com);
uint8_t k_dev_serial_received(uint16_t port);
char    k_dev_serial_read(uint16_t port);
uint8_t k_dev_serial_is_transmit_empty(uint16_t port);
void    k_dev_serial_putchar(uint16_t port, char a);
void    k_dev_serial_write(uint16_t port, char* buff, uint32_t size);
void    k_dev_serial_putstr(uint16_t port, const char* buff);

#endif

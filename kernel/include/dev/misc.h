#ifndef __K_DEV_MISC_H
#define __K_DEV_MISC_H 1

void k_dev_init_null();
void k_dev_init_random();

#define k_dev_init_misc() \
    k_dev_init_null(); \
    k_dev_init_random();

#endif

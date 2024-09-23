global k_dev_read_tsc
k_dev_read_tsc:
    rdtsc
    shl rdx, 32
    or  rax, rdx
    ret

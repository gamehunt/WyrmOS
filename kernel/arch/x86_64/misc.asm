global arch_get_ticks
arch_get_ticks:
    rdtsc
    shl rax, 32
    mov eax, edx
    ret

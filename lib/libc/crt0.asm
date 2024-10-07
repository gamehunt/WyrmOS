extern main
extern __libc_init

section .text
global _start
_start:
    mov rbp, 0
    push rbp

    mov rcx, main
    call __libc_init

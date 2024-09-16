extern main
extern exit

section .text
global _start
_start:
    mov rbp, 0
    push rbp

    call main
    
    mov rdi, rax
    call exit

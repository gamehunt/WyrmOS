global __save_ctx ; context* ctx
__save_ctx:
    mov [rdi + 0], rsp ; Save ESP
    mov [rdi + 8], rbp ; Save EBP

    mov rax, [rsp]      ; Get EIP
    mov [rdi + 16], rax ; Save EIP

    xor rax, rax       ; return 0
	ret

global __load_ctx ; context* ctx
__load_ctx:
    mov rsp, [rdi + 0] ; Restore ESP
    mov rbp, [rdi + 8] ; Restore EBP

    mov  rax, 1     ; Make save function return 1
    jmp [rdi + 16]  ; Jump

global __usr_jmp ; uintptr_t entrypoint, uintptr_t stack
__usr_jmp:
	push   0x23 ; ss
	push   rsi  ; rsp
	push   0x200200  ; rflags (INT | CPUID)
	push   0x1b ; cs
	push   rdi  ; rip
	iretq

global __set_core_base
__set_core_base:
    mov rax, rdi
    mov rdx, rax
    shr rdx, 32
    mov ecx, 0xC0000101
    wrmsr
    mov ecx, 0xC0000102
    wrmsr
    swapgs
    ret


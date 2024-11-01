global __setup_flags
__setup_flags:
    mov rax, cr0
    or  rax, (1 << 1)
    xor rax, (1 << 2)
    xor rax, (1 << 16)
    mov cr0, rax
    mov rax, cr4
    or  rax, (1 << 9)
    or  rax, (1 << 10)
    mov cr4, rax
    ret

global __get_pml
__get_pml:
	mov rax, cr3
	or  rax, rdi
	ret

global __set_pml
__set_pml:
	mov cr3, rdi
	ret

global __get_pagefault_address
__get_pagefault_address:
	mov rax, cr2
	ret

global __invlpg
__invlpg:
    invlpg [rdi]
    ret

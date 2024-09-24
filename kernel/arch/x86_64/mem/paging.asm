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
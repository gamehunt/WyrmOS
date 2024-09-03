global load_interrupt_table
load_interrupt_table:
	lidt [rdi]
	ret

align 4

%macro ISR 1
	global isr%1
	isr%1:
		push 0x0
		push %1
		jmp interrupt_stub
%endmacro

%macro ERROR 1
	global isr%1
	isr%1:
		push %1
		jmp interrupt_stub
%endmacro

ISR 	0
ISR 	1
ISR     2
ISR 	3
ISR 	4
ISR 	5
ISR 	6
ISR 	7
ERROR   8
ISR     9
ERROR   10
ERROR   11
ERROR   12
ERROR   13
ERROR   14
ISR 	15
ISR 	16
ERROR   17
ISR 	18
ISR 	19
ISR 	20
ERROR   21
ISR 	22
ISR 	23
ISR 	24
ISR 	25
ISR 	26
ISR 	27
ISR 	28
ERROR   29
ERROR   30
ISR 	31
; IRQs
ISR 	32 ; 0
ISR 	33
ISR 	34
ISR 	35
ISR 	36
ISR 	37
ISR 	38
ISR 	39
ISR 	40 ; 8
ISR 	41
ISR 	42
ISR 	43
ISR 	44
ISR 	45
ISR 	46
ISR 	47 ; 16

extern __dispatch_interrupt
interrupt_stub:
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	cld

	mov rdi, rsp
	call __dispatch_interrupt

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax

	add rsp, 16

	iretq

global ring3_jump
ring3_jump:
	mov ax, 0x23 ; User Data | 3
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov rax, rsp
	push 0x23 ; User Data | 3
	push rax
	pushf
	push 0x1b ; User Code | 3

	push rdi ; Entry

	iretq

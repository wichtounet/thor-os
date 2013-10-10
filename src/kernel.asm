[BITS 16]

jmp _start

%include "src/utils/intel_16.asm"

_start:
    ; Set stack space
	mov ax, 0x9000
	mov ss, ax
	mov sp, 0xffff

    ; Set data segment
	mov ax, 0x100
	mov ds, ax

    call new_line_16

	mov si, kernel_header_0
	call print_line_16

    mov si, kernel_header_1
	call print_line_16

    mov si, kernel_header_2
	call print_line_16

    call new_line_16

    jmp $

; Datas

	kernel_header_0 db '******************************', 0
	kernel_header_1 db 'Welcome to Thor OS!', 0
	kernel_header_2 db '******************************', 0

    ; Fill the sector (not necessary, but cleaner)
	times 512-($-$$) db 0

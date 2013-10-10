jmp _start

[BITS 16]

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


; Functions

new_line_16:
	mov ah, 0Eh

    mov al, 0Ah
    int 10h

    mov al, 0Dh
    int 10h

    ret

print_line_16:
	mov ah, 0Eh

.repeat:
	lodsb
	cmp al, 0
	je .done
	int 10h
	jmp .repeat

.done:
    call new_line_16

    ret

; Datas

	kernel_header_0 db '******************************', 0
	kernel_header_1 db 'Welcome to Thor OS!', 0
	kernel_header_2 db '******************************', 0

    ; Fill the sector
	times 512-($-$$) db 0

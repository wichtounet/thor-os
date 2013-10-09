	[BITS 16]

; Start in real mode
rm_start:

; 1. Set stable environment

    ; Set stack space (4K) and stack segment
	mov ax, 07C0h
	add ax, 288
	mov ss, ax
	mov sp, 4096

    ; Set data segment
	mov ax, 07C0h
	mov ds, ax

; 2. Welcome the user to the bootloader

    call new_line

	mov si, header_0
	call print_line

    mov si, header_1
	call print_line

    mov si, header_2
	call print_line

    call new_line

; 3. Get ready for protected mode

; TODO

    ; Infinite loop to not exit directly the system
	jmp $

; Functions

new_line:
	mov ah, 0Eh

    mov al, 0Ah
    int 10h

    mov al, 0Dh
    int 10h

    ret

print_line:
	mov ah, 0Eh

.repeat:
	lodsb
	cmp al, 0
	je .done
	int 10h
	jmp .repeat

.done:
    call new_line

    ret

; Datas

	header_0 db '******************************', 0
	header_1 db 'Welcome to Thor OS Bootloader!', 0
	header_2 db '******************************', 0

    ; Make a real bootsector
	times 510-($-$$) db 0
	dw 0xAA55
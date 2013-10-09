	[BITS 16]

start:

; 1. Set stable environment

	mov ax, 07C0h   ; Set up 4K stack space after this bootloader
	add ax, 288     ; (4096 + 512) / 16 bytes per paragraph
	mov ss, ax
	mov sp, 4096

	mov ax, 07C0h   ; Set data segment to where we're loaded
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
	lodsb			; Get character from string
	cmp al, 0
	je .done		; If char is zero, end of string
	int 10h			; Otherwise, print it
	jmp .repeat

.done:
    call new_line

    ret

; Datas

	header_0 db '******************************', 0
	header_1 db 'Welcome to Thor OS Bootloader!', 0
	header_2 db '******************************', 0

	times 510-($-$$) db 0	; Pad remainder of boot sector with 0s
	dw 0xAA55		; The standard PC boot signature
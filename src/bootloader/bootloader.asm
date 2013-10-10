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

    mov si, press_key_msg
    call print_line

    call new_line

    ; Enable A20 gate

    in 		al, 0x92
    or 		al, 2
    out		 0x92, al

    ; Wait for any key
    call key_wait

    mov si, load_kernel
	call print_line

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

key_wait:
    mov		al, 0xD2
    out		64h, al

    mov		al, 0x80
	out		60h, al

    keyup:
		in		al, 0x60
		and	 	al, 10000000b
	jnz		keyup
	Keydown:
	in		al, 0x60

    ret

; Datas

	header_0 db '******************************', 0
	header_1 db 'Welcome to Thor OS Bootloader!', 0
	header_2 db '******************************', 0

    press_key_msg db 'Press any key to load the kernel...', 0
    load_kernel db 'Attempt to load the kernel...', 0

    ; Make a real bootsector
	times 510-($-$$) db 0
	dw 0xAA55
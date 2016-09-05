;=======================================================================
; Copyright Baptiste Wicht 2013-2016.
; Distributed under the terms of the MIT License.
; (See accompanying file LICENSE or copy at
;  http://www.opensource.org/licenses/MIT_1_0.txt)
;=======================================================================

[BITS 16]

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
    test al, al
	je .done
	int 10h
	jmp .repeat

.done:
    call new_line_16

    ret

print_16:
	mov ah, 0Eh

.repeat:
	lodsb
    test al, al
	je .done
	int 10h
	jmp .repeat

.done:
    ret

print_int_16:
    push ax
    push bx
    push dx
    push si

    mov ax, di

    xor si, si

    .loop:
        xor dx, dx
        mov bx, 10
        div bx
        add dx, 48

        push dx
        inc si

        test ax, ax
        jne .loop

    .next:
        test si, si
        je .exit
        dec si

        ; write the char
        pop ax

        mov ah, 0Eh
        int 10h

        jmp .next

    .exit:
        pop si
        pop dx
        pop bx
        pop ax

        ret

key_wait:
    mov al, 0xD2
    out 64h, al

    mov al, 0x80
    out 60h, al

    .keyup:
        in al, 0x60
        and al, 10000000b
    jnz .keyup

    .keydown:
    in al, 0x60

    ret

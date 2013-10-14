; Variables

    current_line dq 0
    current_column dq 0

; Constants

    TRAM equ 0x0B8000 ; Text RAM
    VRAM equ 0x0A0000 ; Video RAM

; Set rdi to the current position based on current_line and current_column
set_current_position:
    push rax
    push rbx
    push rdx

    ; Line offset
    mov rax, [current_line]
    mov rbx, 0x14 * 8
    mul rbx

    ; Column offset
    mov rbx, [current_column]
    shl rbx, 1

    lea rdi, [rax + rbx + TRAM]

    pop rdx
    pop rbx
    pop rax

    ret

goto_next_line:
    push rax

    ; Go to the next line
    mov rax, [current_line]
    inc rax
    mov [current_line], rax

    ; Start at the first column
    mov qword [current_column], 0

    pop rax

    ret

; Print the given string at the current position and update
; the current position for later print
; r8 = string to print
; r9 = length of the string to print
print_normal:
    push rax
    push rbx
    push rdx
    push rdi

    call set_current_position
    mov rbx, r8
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_string

    mov rax, [current_column]
    add rax, r9
    mov [current_column], rax

    pop rdi
    pop rdx
    pop rbx
    pop rax

    ret

; rbx = address of string to print
; rdi = START of write
; dl = code style
print_string:
    push rax

.repeat:
    ; Read the char
    mov al, [rbx]

    ; Test if end of string
    cmp al, 0
    je .done

    ; Write char
    stosb

    ; Write style code
    mov al, dl
    stosb

    inc rbx

    jmp .repeat

.done:
    pop rax

    ret

; Print the given integer to the console
; r8 = integer to print
; rdi = START of write
; dl = code style
print_int:
    push rax
    push rbx
    push rdx
    push r10
    push rsi

    mov rax, r8
    mov r10, rdx

    xor rsi, rsi

    .loop:
        xor rdx, rdx
        mov rbx, 10
        div rbx
        add rdx, 48

        push rdx
        inc rsi

        cmp rax, 0
        jne .loop

    .next:
        cmp rsi, 0
        je .exit
        dec rsi

        ; write the char
        pop rax
        stosb

        ; Write style code
        mov rdx, r10
        mov al, dl
        stosb

        jmp .next

    .exit:
        pop rsi
        pop r10
        pop rdx
        pop rbx
        pop rax

        ret

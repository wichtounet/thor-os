%include "src/utils/utils.asm"
%include "src/utils/keyboard.asm"

%include "src/commands.asm"

; Entry point of the shell, this function never returns
shell_start:
    ; Clean up all the screen
    call clear_command

    call goto_next_line

    ;Display the command line
    mov r8, command_line
    mov r9, command_line_length
    call print_normal

    mov r8, 1
    mov r9, key_entered
    call register_irq_handler

    ret

key_entered:
    in al, 0x60
    mov dl, al
    and dl, 10000000b

    jnz .end_handler

    ; ENTER key
    cmp al, 0x1C
    je .enter

    cmp al, 0x0E
    je .backspace

    call key_to_ascii

    ; Store the entered character
    mov r8, [current_input_length]
    mov byte [current_input_str + r8], al
    inc r8
    mov [current_input_length], r8

    ; Print back the entered char
    call set_current_position
    stosb

    ; Go to the next column
    mov r13, [current_column]
    inc r13
    mov [current_column], r13

    jmp .end_handler

    .enter:
        mov r8, [current_input_length]

        ; If the user didn't enter anything, just go to the next line
        test r8, r8
        je .end

        call goto_next_line

        ; zero terminate the input string
        mov byte [current_input_str + r8], 0

        ; Iterate through the command table and compare each string

        mov r8, [command_table] ; Number of commands
        xor r9, r9              ; iterator

        .start:
            cmp r9, r8
            je .command_not_found

            mov rsi, current_input_str
            mov r10, r9
            shl r10, 4
            mov rdi, [r10 + command_table + 8]

        .next_char:
            mov al, [rsi]
            mov bl, [rdi]

            test al, al
            jne .compare

            test bl, bl
            jne .compare

            ; both == 0

            mov r10, r9
            inc r10
            shl r10, 4

            call [command_table + r10]

            jmp .end

            .compare:

            test al, al
            je .next_command

            test bl, bl
            je .next_command

            cmp al, bl
            jne .next_command

            inc rsi
            inc rdi

            jmp .next_char

        .next_command:
            inc r9
            jmp .start

        .command_not_found:
            mov r8, unknown_command_str_1
            mov r9, unknown_command_str_1_length
            call print_normal

            call set_current_position

            mov rbx, current_input_str
            mov dl, STYLE(BLACK_F, WHITE_B)
            call print_string

            mov rax, [current_column]
            mov rbx, [current_input_length]
            add rax, rbx
            mov [current_column], rax

            mov r8, unknown_command_str_2
            mov r9, unknown_command_str_2_length
            call print_normal

        .end:
            mov qword [current_input_length], 0

            call goto_next_line

            ;Display the command line
            mov r8, command_line
            mov r9, command_line_length
            call print_normal

            jmp .end_handler

    .backspace:

    ; If there are no characters, there is nothing to do
    mov r8, [current_input_length]
    test r8, r8
    je .end_handler

    ; There is one less character on the input
    dec r8
    mov [current_input_length], r8

    ; Go to the previous column
    mov r13, [current_column]
    dec r13
    mov [current_column], r13

    ; Print back the entered char
    call set_current_position
    mov al, ' '
    stosb

    .end_handler:

    ret

; Variables

    current_input_length dq 0
    current_input_str:
        times 32 db 0

; Strings

    header_title db "                                    THOR OS                                     ", 0

    STRING command_line, "thor> "

    STRING unknown_command_str_1, 'The command "'
    STRING unknown_command_str_2, '" does not exist'

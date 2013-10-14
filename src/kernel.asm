[BITS 16]
[ORG 0x1000]

jmp _start

%define BLACK_F 0x0
%define BLUE_F 0x1
%define GREEN_F 0x2
%define CYAN_F 0x3
%define RED_F 0x4
%define PINK_F 0x5
%define ORANGE_F 0x6
%define WHITE_F 0x7

%define BLACK_B 0x0
%define BLUE_B 0x1
%define GREEN_B 0x2
%define CYAN_B 0x3
%define RED_B 0x4
%define PINK_B 0x5
%define ORANGE_B 0x6
%define WHITE_B 0x7

%define STYLE(f,b) ((f << 4) + b)

%macro PRINT_NORMAL 2
    call set_current_position
    mov rbx, %1
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_string

    mov rax, [current_column]
    add rax, %2
    mov [current_column], rax
%endmacro

_start:
    ; Reset data segments because the bootloader set it to
    ; a value incompatible with the kernel
    xor ax, ax
    mov ds, ax

    ; Disable interrupts
    cli

    ; Load GDT
    lgdt [GDT64]

    ; Switch to protected mode
    mov eax, cr0
    or al, 1b
    mov cr0, eax

    ; Desactivate pagination
    mov eax, cr0
    and eax, 01111111111111111111111111111111b
    mov cr0, eax

    jmp (CODE_SELECTOR-GDT64):pm_start

[BITS 32]

pm_start:

    ; Update segments
    mov ax, DATA_SELECTOR-GDT64
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Activate PAE
    mov eax, cr4
    or eax, 100000b
    mov cr4, eax

    ; Clean pages
    mov edi, 0x70000
    mov ecx, 0x10000
    xor eax, eax
    rep stosd

    ; Update pages
    mov dword [0x70000], 0x71000 + 7    ; first PDP table
    mov dword [0x71000], 0x72000 + 7    ; first page directory
    mov dword [0x72000], 0x73000 + 7    ; first page table

    mov edi, 0x73000                    ; address of first page table
    mov eax, 7
    mov ecx, 256                        ; number of pages to map (1 MB)

    make_page_entries:
        stosd
        add     edi, 4
        add     eax, 0x1000
        loop    make_page_entries

    ; Update MSR (Model Specific Registers)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 100000000b
    wrmsr

    ; Copy PML4 address into cr3
    mov eax, 0x70000    ; Bass address of PML4
    mov cr3, eax        ; load page-map level-4 base

    ; Switch to long mode
    mov eax, cr0
    or eax, 10000000000000000000000000000000b
    mov cr0, eax

    jmp (LONG_SELECTOR-GDT64):lm_start

[BITS 64]

lm_start:
    ; Clean up all the screen
    call clear_screen

    .start_waiting:
        call key_wait

        ; ENTER key
        cmp al, 28
        je .new_command

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

        ; Wait for the next key again
        jmp .start_waiting

    .new_command:
        call goto_next_line

        ; zero terminate the input string
        mov r8, [current_input_length]
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

            cmp al, 0
            jne .compare

            cmp bl, 0
            jne .compare

            ; both == 0

            mov r10, r9
            inc r10
            shl r10, 4

            call [command_table + r10]

            jmp .end

            .compare:

            cmp al, 0
            je .next_command

            cmp bl, 0
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
            PRINT_NORMAL unknown_command_str_1, unknown_command_str_1_length

            call set_current_position

            mov rbx, current_input_str
            mov dl, STYLE(BLACK_F, WHITE_B)
            call print_string

            mov rax, [current_column]
            mov rbx, [current_input_length]
            add rax, rbx
            mov [current_column], rax

            PRINT_NORMAL unknown_command_str_2, unknown_command_str_2_length

        .end:
            mov qword [current_input_length], 0

            call goto_next_line

            ;Display the command line
            PRINT_NORMAL command_line, command_line_length

            jmp .start_waiting

; Functions

; Set rdi to the current position based on current_line and current_column
set_current_position:
    push rax
    push rbx

    ; Line offset
    mov rax, [current_line]
    mov rbx, 0x14 * 8
    mul rbx

    ; Column offset
    mov rbx, [current_column]
    shl rbx, 1

    lea rdi, [rax + rbx + TRAM]

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

; In: key in al
; Out: ascci key in al
key_to_ascii:
    and eax, 0xFF
    mov al, [eax + azerty]

    ret

; Return the keyboard input into al
key_wait:
    mov al, 0xD2
    out 0x64, al

    mov al, 0x80
    out 0x60, al

    .key_up:
        in al, 0x60
        and al, 10000000b
    jnz .key_up

    ; key_down
        in al, 0x60

    ret

clear_screen:
    ; Print top bar
    call set_current_position
    mov rbx, header_title
    mov dl, STYLE(WHITE_F, BLACK_B)
    call print_string

    ; Fill the entire screen with black
    mov rdi, TRAM + 0x14 * 8
    mov rcx, 0x14 * 24
    mov rax, 0x0720072007200720
    rep stosq

    ; Line 0 is for header
    mov qword [current_line], 1

    ;Display the command line
    PRINT_NORMAL command_line, command_line_length

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

; Compute the length of string representation of the integer
; in r8 = integer to print
; out rax = string length of int
int_str_length:
    push rbx
    push rdx
    push rsi

    mov rax, r8

    xor rsi, rsi

    .loop:
        xor rdx, rdx
        mov rbx, 10
        div rbx
        add rdx, 48

        inc rsi

        cmp rax, 0
        jne .loop

    .exit:
        mov rax, rsi

        pop rsi
        pop rdx
        pop rbx

        ret

sysinfo_command:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    push rax
    push rbx
    push rcx
    push rdx

    PRINT_NORMAL sysinfo_vendor_id, sysinfo_vendor_id_length

    xor eax, eax
    cpuid

    mov [rsp+0], ebx
    mov [rsp+4], edx
    mov [rsp+8], ecx

    call set_current_position
    mov rbx, rsp
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_string

    call goto_next_line
    PRINT_NORMAL sysinfo_stepping, sysinfo_stepping_length

    mov eax, 1
    cpuid

    mov r15, rax

    mov r8, r15
    and r8, 0xF

    call set_current_position
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_int

    call goto_next_line
    PRINT_NORMAL sysinfo_model, sysinfo_model_length

    ; model id
    mov r14, r15
    and r14, 0xF0

    ; family id
    mov r13, r15
    and r13, 0xF00

    ; extended model id
    mov r12, r15
    and r12, 0xF0000

    ; extended family id
    mov r11, r15
    and r11, 0xFF00000

    shl r12, 4
    mov r8, r14
    add r8, r12
    call set_current_position
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_int

    call goto_next_line
    PRINT_NORMAL sysinfo_family, sysinfo_family_length

    mov r8, r13
    add r8, r11
    call set_current_position
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_int

    call goto_next_line
    PRINT_NORMAL sysinfo_features, sysinfo_features_length

    mov eax, 1
    cpuid

    .mmx:

    mov r15, rdx
    and r15, 1 << 23
    cmp r15, 0
    je .sse

    PRINT_NORMAL sysinfo_mmx, sysinfo_mmx_length

    .sse:

    mov r15, rdx
    and r15, 1 << 25
    cmp r15, 0
    je .sse2

    PRINT_NORMAL sysinfo_sse, sysinfo_sse_length

    .sse2:

    mov r15, rdx
    and r15, 1 << 26
    cmp r15, 0
    je .ht

    PRINT_NORMAL sysinfo_sse2, sysinfo_sse2_length

    .ht:

    mov r15, rdx
    and r15, 1 << 28
    cmp r15, 0
    je .sse3

    PRINT_NORMAL sysinfo_ht, sysinfo_ht_length

    .sse3:

    mov r15, rcx
    and r15, 1 << 9
    cmp r15, 0
    je .sse4_1

    PRINT_NORMAL sysinfo_sse3, sysinfo_sse3_length

    .sse4_1:

    mov r15, rcx
    and r15, 1 << 19
    cmp r15, 0
    je .sse4_2

    PRINT_NORMAL sysinfo_sse4_1, sysinfo_sse4_1_length

    .sse4_2:

    mov r15, rcx
    and r15, 1 << 20
    cmp r15, 0
    je .last

    PRINT_NORMAL sysinfo_sse4_2, sysinfo_sse4_2_length

    .last:

    pop rdx
    pop rcx
    pop rbx
    pop rax

    sub rsp, 16
    leave
    ret

reboot_command:
    ; Reboot using the 8042 keyboard controller
    ; by pulsing the CPU's reset pin
    in al, 0x64
    or al, 0xFE
    out 0x64, al
    mov al, 0xFE
    out 0x64, al

    ; Should never get here
    ret

; Variables

    current_line dq 0
    current_column dq 0

    current_input_length dq 0
    current_input_str:
        times 32 db 0

; Command table

command_table:
    dq 2 ; Number of commands

    dq sysinfo_command_str
    dq sysinfo_command

    dq reboot_command_str
    dq reboot_command

; Strings

%macro STRING 2
    %1 db %2, 0
    %1_length equ $ - %1 - 1
%endmacro

    header_title db "                                    THOR OS                                     ", 0

    sysinfo_command_str db 'sysinfo', 0
    reboot_command_str db 'reboot', 0

    STRING command_line, "thor> "

    STRING unknown_command_str_1, 'The command "'
    STRING unknown_command_str_2, '" does not exist'

    STRING sysinfo_vendor_id, "Vendor ID: "
    STRING sysinfo_stepping, "Stepping: "
    STRING sysinfo_model, "Model: "
    STRING sysinfo_family, "Family: "
    STRING sysinfo_features, "Features: "
    STRING sysinfo_mmx, "mmx "
    STRING sysinfo_sse, "sse "
    STRING sysinfo_sse2, "sse2 "
    STRING sysinfo_sse3, "sse3 "
    STRING sysinfo_sse4_1, "sse4_1 "
    STRING sysinfo_sse4_2, "sse4_2 "
    STRING sysinfo_ht, "ht "

; Constants

    TRAM equ 0x0B8000 ; Text RAM
    VRAM equ 0x0A0000 ; Video RAM

; Qwertz table

azerty:
    db '0',0xF,'1234567890',0xF,0xF,0xF,0xF
    db 'qwertzuiop'
    db '^$',0xD,0x11
    db 'asdfghjklÃ©'
    db 'ù²','*'
    db 'yxcvbnm,;:!'
    db 0xF,'*',0x12,0x20,0xF,0xF

; Global Descriptors Table

GDT64:
    NULL_SELECTOR:
        dw GDT_LENGTH   ; limit of GDT
        dw GDT64        ; linear address of GDT
        dd 0x0

    CODE_SELECTOR:          ; 32-bit code selector (ring 0)
        dw 0x0FFFF
        db 0x0, 0x0, 0x0
        db 10011010b
        db 11001111b
        db 0x0

    DATA_SELECTOR:          ; flat data selector (ring 0)
        dw  0x0FFFF
        db  0x0, 0x0, 0x0
        db  10010010b
        db  10001111b
        db  0x0

    LONG_SELECTOR:  ; 64-bit code selector (ring 0)
        dw  0x0FFFF
        db  0x0, 0x0, 0x0
        db  10011010b       ;
        db  10101111b
        db  0x0

   GDT_LENGTH:

   ; Fill the sector (not necessary, but cleaner)
   times 4196-($-$$) db 0

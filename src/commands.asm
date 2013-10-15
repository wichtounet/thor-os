; String constants

sysinfo_command_str db 'sysinfo', 0
reboot_command_str db 'reboot', 0
clear_command_str db 'clear', 0
help_command_str db 'help', 0

STRING sysinfo_vendor_id, "Vendor ID: "
STRING sysinfo_stepping, "Stepping: "
STRING sysinfo_model, "Model: "
STRING sysinfo_family, "Family: "
STRING sysinfo_features, "Features: "
STRING sysinfo_cpu_brand, "CPU Brand: "
STRING sysinfo_max_frequency, "Max Frequency: "
STRING sysinfo_current_frequency, "Current Frequency: "
STRING sysinfo_l2, "L2 Cache Size: "
STRING sysinfo_mmx, "mmx "
STRING sysinfo_sse, "sse "
STRING sysinfo_sse2, "sse2 "
STRING sysinfo_sse3, "sse3 "
STRING sysinfo_sse4_1, "sse4_1 "
STRING sysinfo_sse4_2, "sse4_2 "
STRING sysinfo_ht, "ht "

STRING available_commands, "Available commands: "
STRING tab, "  "

; Command table

command_table:
    dq 4 ; Number of commands

    dq sysinfo_command_str
    dq sysinfo_command

    dq reboot_command_str
    dq reboot_command

    dq clear_command_str
    dq clear_command

    dq help_command_str
    dq help_command

; Command functions

sysinfo_command:
    push rbp
    mov rbp, rsp
    sub rsp, 20

    push rax
    push rbx
    push rcx
    push rdx
    push r10

    ; Vendor ID

    mov r8, sysinfo_vendor_id
    mov r9, sysinfo_vendor_id_length
    call print_normal

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

    ; CPU Brand String

    mov r8, sysinfo_cpu_brand
    mov r9, sysinfo_cpu_brand_length
    call print_normal

    xor r10, r10

    .next:
    mov rax, 0x80000002
    add rax, r10
    cpuid

    mov [rsp+0], eax
    mov [rsp+4], ebx
    mov [rsp+8], ecx
    mov [rsp+12], edx

    mov r8, rsp
    mov r9, 16
    call print_normal

    inc r10
    cmp r10, 3
    jne .next

    call goto_next_line

    ; Stepping

    mov r8, sysinfo_stepping
    mov r9, sysinfo_stepping_length
    call print_normal

    mov eax, 1
    cpuid

    mov r15, rax

    mov r8, r15
    and r8, 0xF

    call set_current_position
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_int

    call goto_next_line
    mov r8, sysinfo_model
    mov r9, sysinfo_model_length
    call print_normal

    ; model id
    mov r14, r15
    and r14, 0xF0
    shr r14, 4

    ; family id
    mov r13, r15
    and r13, 0xF00
    shr r13, 8

    ; extended model id
    mov r12, r15
    and r12, 0xF0000
    shr r12, 12

    ; extended family id
    mov r11, r15
    and r11, 0xFF00000
    shr r11, 16

    mov r8, r14
    add r8, r12
    call set_current_position
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_int

    call goto_next_line
    mov r8, sysinfo_family
    mov r9, sysinfo_family_length
    call print_normal

    mov r8, r13
    add r8, r11
    call set_current_position
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_int

    call goto_next_line
    mov r8, sysinfo_features
    mov r9, sysinfo_features_length
    call print_normal

    mov eax, 1
    cpuid

    .mmx:

    mov r15, rdx
    and r15, 1 << 23
    cmp r15, 0
    je .sse

    mov r8, sysinfo_mmx
    mov r9, sysinfo_mmx_length
    call print_normal

    .sse:

    mov r15, rdx
    and r15, 1 << 25
    cmp r15, 0
    je .sse2

    mov r8, sysinfo_sse
    mov r9, sysinfo_sse_length
    call print_normal

    .sse2:

    mov r15, rdx
    and r15, 1 << 26
    cmp r15, 0
    je .ht

    mov r8, sysinfo_sse2
    mov r9, sysinfo_sse2_length
    call print_normal

    .ht:

    mov r15, rdx
    and r15, 1 << 28
    cmp r15, 0
    je .sse3

    mov r8, sysinfo_ht
    mov r9, sysinfo_ht_length
    call print_normal

    .sse3:

    mov r15, rcx
    and r15, 1 << 9
    cmp r15, 0
    je .sse4_1

    mov r8, sysinfo_sse3
    mov r9, sysinfo_sse3_length
    call print_normal

    .sse4_1:

    mov r15, rcx
    and r15, 1 << 19
    cmp r15, 0
    je .sse4_2

    mov r8, sysinfo_sse4_1
    mov r9, sysinfo_sse4_1_length
    call print_normal

    .sse4_2:

    mov r15, rcx
    and r15, 1 << 20
    cmp r15, 0
    je .frequency

    mov r8, sysinfo_sse4_2
    mov r9, sysinfo_sse4_2_length
    call print_normal

    .frequency:

    call goto_next_line

    mov r8, sysinfo_max_frequency
    mov r9, sysinfo_max_frequency_length
    call print_normal

    mov eax, 0x80000004
    cpuid

    mov [rsp+0], eax
    mov [rsp+4], ebx
    mov [rsp+8], ecx
    mov [rsp+12], edx

    mov rax, rsp

    .next_char:
        mov bl, [rax]
        inc rax
        cmp bl, 0
        jne .next_char

    xor rbx, rbx
    xor rcx, rcx

    mov cl, [rax - 5]
    sub rcx, 48
    imul rcx, 10
    add rbx, rcx

    mov cl, [rax - 6]
    sub rcx, 48
    imul rcx, 100
    add rbx, rcx

    movzx rcx, byte [rax - 8]
    sub rcx, 48
    imul rcx, 1000
    add rbx, rcx

    call set_current_position
    mov r8, rbx
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_int

    call goto_next_line

    mov r8, sysinfo_current_frequency
    mov r9, sysinfo_current_frequency_length
    call print_normal

    ; rbx = max_frequency

    xor rax, rax
    mov ax, cs
    and ax, 11b

    cmp ax, 0
    jne .last

    ; read MPERF
    mov ecx, 0xe7
    rdmsr

    .last:

    call goto_next_line

    mov r8, sysinfo_l2
    mov r9, sysinfo_l2_length
    call print_normal

    xor rcx, rcx
    mov eax, 0x80000006
    cpuid

    and ecx, 0xFFFF0000
    shr ecx, 16

    call set_current_position
    mov r8, rcx
    mov dl, STYLE(BLACK_F, WHITE_B)
    call print_int

    pop r10
    pop rdx
    pop rcx
    pop rbx
    pop rax

    sub rsp, 20
    leave
    ret

help_command:
    push r8
    push r9
    push r10
    push r11
    push r12

    mov r8, available_commands
    mov r9, available_commands_length
    call print_normal

    mov r12, [command_table]   ; Number of commands
    xor r11, r11              ; iterator

    .start:
        cmp r11, r12
        je .end

        mov r10, r11
        shl r10, 4

        call goto_next_line

        mov r8, tab
        mov r9, tab_length
        call print_normal

        mov r8, [r10 + command_table + 8]
        mov r9, 1
        call print_normal

        inc r11
        jmp .start

    .end:

    pop r12
    pop r11
    pop r10
    pop r9
    pop r8

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

clear_command:
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
    mov qword [current_line], 0
    mov qword [current_column], 0

    ret


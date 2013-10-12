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

%macro PRINT_B 3
    mov rdi, TRAM
    mov rbx, %1
    mov dl, STYLE(%2, %3)
    call print_string
%endmacro

%macro PRINT_P 3
    mov rbx, %1
    mov dl, STYLE(%2, %3)
    call print_string
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
    mov     eax, 0x70000    ; Bass address of PML4
    mov     cr3, eax        ; load page-map level-4 base

    ; Switch to long mode
    mov eax, cr0
    or eax, 10000000000000000000000000000000b
    mov cr0, eax

    jmp (LONG_SELECTOR-GDT64):lm_start

[BITS 64]

lm_start:
    ; Clean up all the screen
    call clear_screen

    ;Display the command line
    mov rdi, TRAM + 0x14 * 8
    PRINT_P command_line, BLACK_F, WHITE_B

    ; Wait
    jmp $

; Functions

clear_screen:
    ; Print top bar
    PRINT_B header_title, WHITE_F, BLACK_B

    mov rdi, TRAM + 0x14 * 8

    mov rcx, 0x14 * 24
    mov rax, 0x0720072007200720
    rep stosq

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

; Datas

    kernel_header_0 db '******************************', 0
    kernel_header_1 db 'Welcome to Thor OS!', 0
    kernel_header_2 db '******************************', 0

    header_title db "                                    THOR OS                                     ", 0
    command_line db "thor> ", 0

; Constants

    TRAM equ 0x0B8000 ; Text RAM
    VRAM equ 0x0A0000 ; Video RAM

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
   times 1024-($-$$) db 0

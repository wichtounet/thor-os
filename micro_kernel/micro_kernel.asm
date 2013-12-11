;=======================================================================
; Copyright Baptiste Wicht 2013.
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;=======================================================================

[BITS 16]

[ORG 0x1000]

jmp _start

e820_mmap:
    pusha

    xor ax, ax
    mov es, ax
    mov di, e820_memory_map

    xor ebx, ebx
    xor bp, bp
    mov edx, 0x0534D4150
    mov eax, 0xe820
    mov [es:di + 20], dword 1
    mov ecx, 24
    int 0x15
    jc .failed

    mov edx, 0x0534D4150
    cmp eax, edx
    jne .failed
    jmp .jmpin

    .e820lp:
    mov eax, 0xE820
    mov [es:di + 20], dword 1
    mov ecx, 24
    int 0x15
    jc .e820f
    mov edx, 0x0534D4150

    .jmpin:
    jcxz .skipent
    cmp cl, 20
    jbe .notext
    test byte [es:di + 20], 1
    je .skipent

    .notext:
    mov ecx, [es:di + 8]
    or ecx, [es:di + 12]
    jz .skipent
    inc bp
    add di, 24

    .skipent:
    test ebx, ebx
    jne .e820lp

    .e820f:
    mov  [e820_entry_count], bp

    clc
    popa
    ret

    .failed:
    stc
    popa
    ret

_start:
    ; Reset data segments because the bootloader set it to
    ; a value incompatible with the kernel
    xor ax, ax
    mov ds, ax

    ; Disable interrupts
    cli

    call e820_mmap
    setc al
    mov [e820_failed], al

    ; Load GDT
    lgdt [GDTR64]

    ; Switch to protected mode
    mov eax, cr0
    or al, 1b
    mov cr0, eax

    ; Disable paging
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
    or eax, 1 << 5
    mov cr4, eax

    ; Configure the Page-Map Level 4 Table (PML4T)

    ; Clear the tables
    mov edi, 0x70000
    mov ecx, 4096
    xor eax, eax
    rep stosd

    ; Update tables (3 means Present, Writable and Supervisor only)
    mov dword [0x70000], 0x71000 + 3   ; PML4T[0] -> PDPT
    mov dword [0x71000], 0x72000 + 3   ; PDPT[0] -> PDT
    mov dword [0x72000], 0x73000 + 3   ; PDT[0] -> PT

    mov edi, 0x73000 ; First PT
    mov ebx, 0x3     ; Present, Writeable and Supervisor only
    mov ecx, 256     ; Map first MiB

    .write_entry:
    mov dword [edi], ebx
    add ebx, 0x1000  ; Map the next block of 4KiB
    add edi, 8       ; A page entry in PT is 64 bit in size
    loop .write_entry

    ; Enable long mode by seting the EFER.LME flag
    mov ecx, 0xC0000080
    rdmsr
    or eax, 100000000b
    wrmsr

    ; Se the address of the page directory
    mov eax, 0x70000    ; Bass address of PML4
    mov cr3, eax        ; load page-map level-4 base

    ; Enable paging
    mov eax, cr0
    or eax, 10000000000000000000000000000000b
    mov cr0, eax

    jmp (LONG_SELECTOR-GDT64):lm_start

[BITS 64]

lm_start:
    ; Go to the kernel
    call 0x5000

    ; Normally should never arrive here
    jmp $

; Functions

; Global Descriptors Table

GDT64:
    NULL_SELECTOR:
        dq 0

    CODE_SELECTOR:          ; 32-bit code selector (ring 0)
        dw 0x0FFFF
        db 0x0, 0x0, 0x0
        db 10011010b
        db 11001111b
        db 0x0

    DATA_SELECTOR:          ; flat data selector (ring 0)
        dw 0x0FFFF
        db 0x0, 0x0, 0x0
        db 10010010b
        db 10001111b
        db 0x0

    LONG_SELECTOR:  ; 64-bit code selector (ring 0)
        dw 0x0FFFF
        db 0x0, 0x0, 0x0
        db 10011010b
        db 10101111b
        db 0x0

GDTR64:
    dw 4 * 8 - 1 ; Length of GDT
    dd GDT64

e820_failed:
    db 0

e820_entry_count:
    dw 0

e820_memory_map:
    times 32 dq 0, 0, 0

; Fill the sector (not necessary, but cleaner)

   times 16384-($-$$) db 0

[BITS 16]

jmp _start

%include "src/utils/intel_16.asm"

_start:
    ; Set stack space
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xffff

    ; Set data segment
    mov ax, 0x100
    mov ds, ax

    call new_line_16

    mov si, kernel_header_0
    call print_line_16

    mov si, kernel_header_1
    call print_line_16

    mov si, kernel_header_2
    call print_line_16

    call new_line_16

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
    mov     dword [0x70000], 0x71000 + 7    ; first PDP table
    mov     dword [0x71000], 0x72000 + 7    ; first page directory
    mov     dword [0x72000], 0x73000 + 7    ; first page table

    mov     edi, 0x73000                    ; address of first page table
    mov     eax, 7
    mov     ecx, 256                        ; number of pages to map (1 MB)

    make_page_entries:
        stosd
        add     edi, 4
        add     eax, 0x1000
        loop    make_page_entries

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

    jmp $

; Datas

    kernel_header_0 db '******************************', 0
    kernel_header_1 db 'Welcome to Thor OS!', 0
    kernel_header_2 db '******************************', 0

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
   times 512-($-$$) db 0

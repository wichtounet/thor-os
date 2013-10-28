[BITS 16]

%ifndef DEBUG
[ORG 0x1000]
%endif

jmp _start

_start:
    ; Reset data segments because the bootloader set it to
    ; a value incompatible with the kernel
    xor ax, ax
    mov ds, ax

    ; Disable interrupts
    cli

    ; Load GDT
    lgdt [GDTR64]

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
    or eax, 1 << 5
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
    ; Install IDT
    call install_idt

    ; Install ISRs
    call install_isrs

    ; Remap IRQs with wrong numbers
    call remap_irqs

    ; Install all IRQs
    call install_irqs

    ; Install custom syscalls
    call install_syscalls

    sti

    call 0x5000

    jmp $

    ; Enter the shell
    ;call shell_start

; Includes

%include "utils/macros.asm"
%include "utils/console.asm"

%include "interrupts.asm"
%include "shell.asm"

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
        dw  0x0FFFF
        db  0x0, 0x0, 0x0
        db  10010010b
        db  10001111b
        db  0x0

    LONG_SELECTOR:  ; 64-bit code selector (ring 0)
        dw  0x0FFFF
        db  0x0, 0x0, 0x0
        db  10011010b
        db  10101111b
        db  0x0

GDTR64:
    dw 4 * 8 - 1 ; Length of GDT
    dd GDT64

; Fill the sector (not necessary, but cleaner)

   times 16384-($-$$) db 0

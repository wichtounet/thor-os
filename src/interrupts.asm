
; Variables

STRING isr0_msg, "Divide by Zero exception "
STRING isr1_msg, "Debug Exception "
STRING isr2_msg, "Non Maskale Interrupt Exception "
STRING isr3_msg, "Breakpoint Exception "
STRING isr4_msg, "Into Detected Overflow Exception "
STRING isr5_msg, "Out Of Bounds Exception "
STRING isr6_msg, "Invalid Opcode Exception "
STRING isr7_msg, "No Coprocessor Exception "
STRING isr8_msg, "Double Fault Exception "
STRING isr9_msg, "Coprocessor Segment Overrun Exception "
STRING isr10_msg, "Bad TSS Exception "
STRING isr11_msg, "Segment Not Present Exception "
STRING isr12_msg, "Stack Fault Exception "
STRING isr13_msg, "General Protection Fault Exception "
STRING isr14_msg, "Page Fault Exception "
STRING isr15_msg, "Unknown Interrupt Exception "
STRING isr16_msg, "Coprocessor Fault Exception "
STRING isr17_msg, "Alignment Check Exception "
STRING isr18_msg, "Machine Check Exception "
STRING isr19_msg, "19: Reserved Exception "
STRING isr20_msg, "20: Reserved Exception "
STRING isr21_msg, "21: Reserved Exception "
STRING isr22_msg, "22: Reserved Exception "
STRING isr23_msg, "23: Reserved Exception "
STRING isr24_msg, "24: Reserved Exception "
STRING isr25_msg, "25: Reserved Exception "
STRING isr26_msg, "26: Reserved Exception "
STRING isr27_msg, "27: Reserved Exception "
STRING isr28_msg, "28: Reserved Exception "
STRING isr29_msg, "29: Reserved Exception "
STRING isr30_msg, "30: Reserved Exception "
STRING isr31_msg, "31: Reserved Exception "

; Routines

%macro CREATE_ISR 1
_isr%1:
    push r8
    push r9

    cli

    mov r8, isr%1_msg
    mov r9, isr%1_msg_length
    call print_normal

    sti

    pop r9
    pop r8

    iretq
%endmacro

%macro IDT_SET_GATE 4

    ; address of the entry
    lea rdi, [IDT64 + %1 * 128]

    mov rax, %2
    mov word [rdi], ax ; base_lo
    mov word [rdi+2], %3 ; selector
    mov byte [rdi+4], 0  ; zero
    mov byte [rdi+5], %4 ; flags

    shr rax, 16
    mov word [rdi+6], ax ; base_middle
    shr rax, 16
    mov dword [rdi+8], eax ; base_hi
    mov dword [rdi+12], 0  ; zero

%endmacro

%assign i 0
%rep 32
CREATE_ISR i
%assign i i+1
%endrep

; Functions

install_idt:
    lidt [IDTR64]

    ret

install_isrs:
    IDT_SET_GATE 0, _isr0, 0x08, 0x8E
    IDT_SET_GATE 1, _isr1, 0x08, 0x8E
    IDT_SET_GATE 2, _isr2, 0x08, 0x8E
    IDT_SET_GATE 3, _isr3, 0x08, 0x8E
    IDT_SET_GATE 4, _isr4, 0x08, 0x8E
    IDT_SET_GATE 5, _isr5, 0x08, 0x8E
    IDT_SET_GATE 6, _isr6, 0x08, 0x8E
    IDT_SET_GATE 7, _isr7, 0x08, 0x8E
    IDT_SET_GATE 8, _isr8, 0x08, 0x8E
    IDT_SET_GATE 9, _isr9, 0x08, 0x8E
    IDT_SET_GATE 10, _isr10, 0x08, 0x8E
    IDT_SET_GATE 11, _isr11, 0x08, 0x8E
    IDT_SET_GATE 12, _isr12, 0x08, 0x8E
    IDT_SET_GATE 13, _isr13, 0x08, 0x8E
    IDT_SET_GATE 14, _isr14, 0x08, 0x8E
    IDT_SET_GATE 15, _isr15, 0x08, 0x8E
    IDT_SET_GATE 16, _isr16, 0x08, 0x8E
    IDT_SET_GATE 17, _isr17, 0x08, 0x8E
    IDT_SET_GATE 18, _isr18, 0x08, 0x8E
    IDT_SET_GATE 19, _isr19, 0x08, 0x8E
    IDT_SET_GATE 20, _isr20, 0x08, 0x8E
    IDT_SET_GATE 21, _isr21, 0x08, 0x8E
    IDT_SET_GATE 22, _isr22, 0x08, 0x8E
    IDT_SET_GATE 23, _isr23, 0x08, 0x8E
    IDT_SET_GATE 24, _isr24, 0x08, 0x8E
    IDT_SET_GATE 25, _isr25, 0x08, 0x8E
    IDT_SET_GATE 26, _isr26, 0x08, 0x8E
    IDT_SET_GATE 27, _isr27, 0x08, 0x8E
    IDT_SET_GATE 28, _isr28, 0x08, 0x8E
    IDT_SET_GATE 29, _isr29, 0x08, 0x8E
    IDT_SET_GATE 30, _isr30, 0x08, 0x8E
    IDT_SET_GATE 31, _isr31, 0x08, 0x8E

    ret

; Data structures

; each idt entry is form like that:
; 16 base_lo
; 16 selector
; 8 zero
; 8 flags
; 16 base_middles
; 32 base_high
; 32 zero

IDT64:
    times 256 dq 0,0

IDTR64:
    dw (256 * 16) - 1  ; Limit
    dq IDT64           ; Base

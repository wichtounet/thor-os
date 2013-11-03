
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

STRING cr2_str, "cr2: "
STRING rsp_str, "rsp: "
STRING error_code_str, "Error code: "
STRING rip_str, "rip: "

; Macros

%macro CREATE_ISR 1
_isr%1:
    ; Disable interruptions to avoid being interrupted
    cli

    mov r10, [rsp] ; error code
    mov r11, [rsp+8] ; saved rip

    lea rdi, [12 * 8 * 0x14 + 30 * 2 + TRAM]
    mov dl, STYLE(RED_F, WHITE_B)
    mov rbx, isr%1_msg
    call print_string

    ; print rip

    lea rdi, [13 * 8 * 0x14 + 30 * 2 + TRAM]
    mov rbx, rip_str
    call print_string

    lea rdi, [13* 8 * 0x14 + 35 * 2 + TRAM]
    mov r8, r11
    call print_int

    ; print rsp

    lea rdi, [14 * 8 * 0x14 + 30 * 2 + TRAM]
    mov rbx, rsp_str
    call print_string

    lea rdi, [14 * 8 * 0x14 + 35 * 2 + TRAM]
    mov r8, rsp
    call print_int

    ; More informations for some specific exceptions

    mov rax, %1
    cmp rax, 14
    jne .end

    .page_fault_exception:

    ; print cr2

    lea rdi, [15 * 8 * 0x14 + 30 * 2 + TRAM]
    mov rbx, cr2_str
    call print_string

    lea rdi, [15 * 8 * 0x14 + 35 * 2 + TRAM]
    mov r8, cr2
    call print_int

    ; print error code

    lea rdi, [16 * 8 * 0x14 + 30 * 2 + TRAM]
    mov rbx, error_code_str
    call print_string

    lea rdi, [16 * 8 * 0x14 + 30 * 2 + TRAM + error_code_str_length * 2]
    mov r8, r10
    call print_int

    .end:

    ; Simply halt the CPU because we don't know how to solve the problem
    hlt
    iretq
%endmacro

%macro CREATE_IRQ 1
_irq%1:
    ; Disable interruptions to avoid being interrupted
    cli

    push rax

    mov rax, [irq_handlers + 8 *%1]

    ; If there are no handler, just send EOI
    test rax, rax
    je .eoi

    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9

    ; Call the handler
    call rax

    push r9
    push r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx

    .eoi:

    mov rax, %1 ; IRQ number
    cmp rax, 8
    jl .master

    ; If IRQ 8 -> 15, send EOI to PIC2

    mov al, 0x20
    out 0xA0, al

    .master:

    ; Send EOI to PIC1
    mov al, 0x20
    out 0x20, al

    pop rax

    iretq
%endmacro

%macro IDT_SET_GATE 4
    ; address of the entry
    lea rdi, [IDT64 + %1 * 16]

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

%assign i 0
%rep 16
CREATE_IRQ i
%assign i i+1
%endrep

; Functions

install_idt:
    lidt [IDTR64]

    ret

install_isrs:
    IDT_SET_GATE 0, _isr0, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 1, _isr1, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 2, _isr2, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 3, _isr3, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 4, _isr4, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 5, _isr5, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 6, _isr6, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 7, _isr7, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 8, _isr8, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 9, _isr9, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 10, _isr10, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 11, _isr11, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 12, _isr12, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 13, _isr13, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 14, _isr14, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 15, _isr15, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 16, _isr16, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 17, _isr17, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 18, _isr18, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 19, _isr19, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 20, _isr20, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 21, _isr21, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 22, _isr22, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 23, _isr23, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 24, _isr24, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 25, _isr25, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 26, _isr26, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 27, _isr27, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 28, _isr28, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 29, _isr29, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 30, _isr30, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 31, _isr31, LONG_SELECTOR-GDT64, 0x8E

    ret

remap_irqs:
    mov al, 0x11
    out 0x20, al ; Restart PIC1
    out 0xA0, al ; Restart PIC2

    mov al, 0x20
    out 0x21, al ; Make PIC1 starts at 32
    mov al, 0x28
    out 0xA1, al ; Make PIC2 starts at 40

    mov al, 0x04
    out 0x21, al ; Setup cascading for PIC1
    mov al, 0x02
    out 0xA1, al ; Setup cascading for PIC2

    mov al, 0x01
    out 0x21, al ; 8086 mode for PIC1
    out 0xA1, al ; 8086 mode for PIC2

    mov al, 0x0
    out 0x21, al ; Enable all IRQs on PIC1
    out 0xA1, al ; Enable all IRQs on PIC2

    ret

install_irqs:
    IDT_SET_GATE 32, _irq0, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 33, _irq1, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 34, _irq2, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 35, _irq3, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 36, _irq4, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 37, _irq5, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 38, _irq6, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 39, _irq7, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 40, _irq8, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 41, _irq9, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 42, _irq10, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 43, _irq11, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 44, _irq12, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 45, _irq13, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 46, _irq14, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 47, _irq15, LONG_SELECTOR-GDT64, 0x8E

    ret

install_syscalls:
    IDT_SET_GATE 60, syscall_reboot, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 61, syscall_irq, LONG_SELECTOR-GDT64, 0x8E
    IDT_SET_GATE 62, syscall_mmap, LONG_SELECTOR-GDT64, 0x8E

    ret

; r8 = irq
; r9 = handler address
register_irq_handler:
    mov [irq_handlers + r8 * 8], r9

    ret

; Syscalls

; r8 = irq
; r9 = handler address
syscall_irq:
    cli

    call register_irq_handler

    iretq

; Reboot the computer
; No parameters
syscall_reboot:
    cli

    push rax

    mov al, 0x64
    or al, 0xFE
    out 0x64, al
    mov al, 0xFE
    out 0x64, al

    pop rax

    iretq

syscall_mmap:
    cli

    .e820_failed:

    cmp r8, 0
    jne .entry_count

    movzx rax, byte [e820_failed]
    iretq

    .entry_count:

    cmp r8, 1
    jne .e820_mmap

    movzx rax, word [e820_entry_count]
    iretq

    .e820_mmap:

    mov rax, e820_memory_map
    iretq

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
    times 128 dq 0,0

IDTR64:
    dw (128* 16) - 1  ; Limit
    dq IDT64           ; Base

; Handlers for each IRQ, 0 indicate no handler
irq_handlers:
    times 16 dq 0

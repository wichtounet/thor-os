;=======================================================================
; Copyright Baptiste Wicht 2013.
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;=======================================================================

[BITS 16]

jmp second_step

%include "intel_16.asm"

; Loaded at 0x900
second_step:
    ; Reset disk drive
    xor ax, ax
    xor ah, ah
    mov dl, 0
    int 0x13

    ; Loading the assembly kernel from floppy

    KERNEL_BASE equ 0x100      ; 0x100:0x0 = 0x1000
    sectors equ 0x48           ; sectors to read
    bootdev equ 0x0

    mov ax, KERNEL_BASE
    mov es, ax
    xor bx, bx

    mov ah, 0x2         ; Read sectors from memory
    mov al, sectors     ; Number of sectors to read
    xor ch, ch          ; Cylinder 0
    mov cl, 3           ; Sector 2
    xor dh, dh          ; Head 0
    mov dl, bootdev     ; Drive
    int 0x13

    jmp dword KERNEL_BASE:0x0

    times 512-($-$$) db 0
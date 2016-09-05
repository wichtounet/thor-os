;=======================================================================
; Copyright Baptiste Wicht 2013-2016.
; Distributed under the terms of the MIT License.
; (See accompanying file LICENSE or copy at
;  http://www.opensource.org/licenses/MIT_1_0.txt)
;=======================================================================

[BITS 16]

jmp rm_start

%include "intel_16.asm"

; Start in real mode
rm_start:
    ; Set stack space (4K) and stack segment
    xor ax, ax
    mov ss, ax
    mov sp, 0x4000

    ; Set data segment
    mov ax, 0x7C0
    mov ds, ax

    ; Hide cursor
    mov ah, 0x01
    mov cx, 0x2607
    int 0x10

    ; Move cursor at top left position
    mov ah, 0x02
    xor bx, bx
    xor dx, dx
    int 0x10

    ; Clear screen
    mov ah, 0x06
    xor al, al
    xor bx, bx
    mov bh, 0x07
    xor cx, cx
    mov dh, 24
    mov dl, 79
    int 0x10

    ; Enable A20 gate
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 2. Welcome the user to the bootloader

    mov si, header_1
    call print_line_16

    ; Check if Extended Read is available
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, 0x80
    int 0x13

    jc extensions_not_supported

    ; Tests

    ; 3. Wait for a key press

    call new_line_16

    mov si, press_key_msg
    call print_line_16

    call new_line_16

    call key_wait

    ; 4. Once the user pressed a key, load the second stage

    mov si, load_msg
    call print_line_16

    ; Loading the stage 2 from disk

    mov ah, 0x42
    mov si, DAP
    mov dl, 0x80
    int 0x13

    jc read_failed

    ; Run the stage 2

    jmp dword 0x410:0x0

extensions_not_supported:
    mov si, extensions_not_supported_msg
    call print_line_16

    jmp error_end

read_failed:
    mov si, read_failed_msg
    call print_line_16

error_end:
    mov si, load_failed
    call print_line_16

    jmp $

; Variable Datas

DAP:
.size       db 0x10
.null       db 0x0
.count      dw 3
.offset     dw 0
.segment    dw 0x410
.lba        dd 1
.lba48      dd 0

; Constants Datas

    header_1 db 'Welcome to Thor OS Bootloader!', 0

    press_key_msg db 'Press any key to load the kernel...', 0
    load_msg db 'Attempt to load the stage 2...', 0

    read_failed_msg db 'Read disk failed', 0
    load_failed db 'Stage 2 loading failed', 0
    extensions_not_supported_msg db 'BIOS Extensions not supported', 0

; Make a real bootsector

    times 446-($-$$) db 0
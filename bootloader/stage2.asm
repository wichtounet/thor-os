;=======================================================================
; Copyright Baptiste Wicht 2013.
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;=======================================================================

[BITS 16]

jmp second_step

%include "intel_16.asm"
%include "sectors.asm"

KERNEL_BASE equ 0x100      ; 0x100:0x0 = 0x1000

DAP:
.size       db 0x10
.null       db 0x0
.count      dw 0
.offset     dw 0
.segment    dw 0x0
.lba        dd 0
.lba48      dd 0

; Loaded at 0x90:0x0
second_step:
    ; Set data segment
    mov ax, 0x90
    mov ds, ax

    ; Used for disk access
    xor ax, ax
    mov gs, ax

    mov si, load_kernel
    call print_line_16

    ; 1. Read the MBR to get partition table

    mov byte [DAP.count], 1
    mov word [DAP.offset], 0x1000
    mov word [DAP.segment], 0
    mov dword [DAP.lba], 0

    mov ah, 0x42
    mov si, DAP
    mov dl, 0x80
    int 0x13

    jc read_failed

    mov ax, [gs:(0x1000 + 446 + 8)]
    mov [partition_start], ax

    ; 2. Read the VBR of the partition to get FAT informations

    mov byte [DAP.count], 1
    mov word [DAP.offset], 0x1000
    mov word [DAP.segment], 0

    mov di, [partition_start]
    mov word [DAP.lba], di

    mov ah, 0x42
    mov si, DAP
    mov dl, 0x80
    int 0x13

    jc read_failed

    mov ah, [gs:(0x1000 + 13)]
    mov [sectors_per_cluster], ah

    mov ax, [gs:(0x1000 + 14)]
    mov [reserved_sectors], ax

    mov ah, [gs:(0x1000 + 16)]
    mov [number_of_fat], ah

    mov ax, [gs:(0x1000 + 36)]
    mov [sectors_per_fat], ax

    mov ax, [gs:(0x1000 + 44)]
    mov [root_dir_start], ax

    ; fat_begin = partition_start + reserved_sectors
    mov ax, [partition_start]
    mov bx, [reserved_sectors]
    add ax, bx
    mov [fat_begin], ax

    ; cluster_begin = (number_of_fat * sectors_per_fat) + fat_begin
    mov ax, [sectors_per_fat]
    movzx bx, [number_of_fat]
    mul bx
    mov bx, [fat_begin]
    add ax, bx
    mov [cluster_begin], ax

    ; entries per cluster = (512/32) * sectors_per_cluster
    mov ax, 32
    movzx bx, byte [sectors_per_cluster]
    mul bx
    mov [entries_per_cluster], ax

    ; 3. Read the root directory to find the kernel executable

    mov ah, [sectors_per_cluster]
    mov byte [DAP.count], ah
    mov word [DAP.offset], 0x1000
    mov word [DAP.segment], 0

    ; Compute LBA from root_dir_start
    mov ax, [root_dir_start]
    sub ax, 2
    movzx bx, byte [sectors_per_cluster]
    mul bx
    mov bx, [cluster_begin]
    add ax, bx

    mov word [DAP.lba], ax

    mov ah, 0x42
    mov si, DAP
    mov dl, 0x80
    int 0x13

    jc read_failed

    mov si, 0x1000
    xor cx, cx

    .next:
        mov ah, [gs:si]

        ; Test if it is the end of the directory
        test ah, ah
        je .end_of_directory

        ; Verify char by char if it is KERNEL.BIN
        cmp ah, 75
        jne .continue

        mov ah, [gs:(si+1)]
        cmp ah, 69
        jne .continue

        mov ah, [gs:(si+2)]
        cmp ah, 82
        jne .continue

        mov ah, [gs:(si+3)]
        cmp ah, 78
        jne .continue

        mov ah, [gs:(si+4)]
        cmp ah, 69
        jne .continue

        mov ah, [gs:(si+5)]
        cmp ah, 76
        jne .continue

        mov ah, [gs:(si+6)]
        cmp ah, 32
        jne .continue

        mov ah, [gs:(si+7)]
        cmp ah, 32
        jne .continue

        mov ah, [gs:(si+8)]
        cmp ah, 66
        jne .continue

        mov ah, [gs:(si+9)]
        cmp ah, 73
        jne .continue

        mov ah, [gs:(si+10)]
        cmp ah, 78
        jne .continue

        ; cluster high
        mov ax, [gs:(si+20)]
        mov [cluster_high], ax

        ; cluster low
        mov ax, [gs:(si+26)]
        mov [cluster_low], ax

        ; file_size low
        mov ax, [gs:(si+28)]
        mov [size_low], ax

        ; file_size high
        mov ax, [gs:(si+30)]
        mov [size_high], ax

        jmp .found

    .continue:
        add si, 32
        inc cx

        mov bx, [entries_per_cluster]
        cmp cx, bx
        jne .next

    .end_of_cluster:
    .end_of_directory:
        mov si, kernel_not_found
        call print_line_16

        jmp error_end

    ; TODO Find kernel.bin in the directory

    .found:

    mov si, kernel_found
    call print_line_16

    jmp $

read_failed:
    mov si, read_failed_msg
    call print_line_16

error_end:
    mov si, load_failed
    call print_line_16

    jmp $

; Variables

    partition_start dw 0
    reserved_sectors dw 0
    number_of_fat db 0
    sectors_per_fat dw 0
    sectors_per_cluster db 0
    root_dir_start dw 0
    entries_per_cluster dw 0
    fat_begin dw 0
    cluster_begin dw 0

    cluster_high dw 0
    cluster_low dw 0
    size_high dw 0
    size_low dw 0

; Constant Datas

    load_kernel db 'Attempt to load the kernel...', 0
    kernel_found db 'Kernel found. Starting kernel loading...', 0
    kernel_not_found db 'Kernel not found...', 0
    kernel_loaded db 'Kernel fully loaded', 0
    star db '*', 0

    read_failed_msg db 'Read disk failed', 0
    load_failed db 'Kernel loading failed', 0

; Make it two sectors exactly

    times 1024-($-$$) db 0
;=======================================================================
; Copyright Baptiste Wicht 2013-2016.
; Distributed under the terms of the MIT License.
; (See accompanying file LICENSE or copy at
;  http://www.opensource.org/licenses/MIT_1_0.txt)
;=======================================================================

[BITS 16]

jmp second_step

%include "intel_16.asm"

FREE_SEGMENT equ 0x5000
FREE_BASE equ 0x4500

KERNEL_BASE equ 0x600      ; 0x600:0x0 (0x6000)

DAP:
.size       db 0x10
.null       db 0x0
.count      dw 0
.offset     dw 0
.segment    dw 0x0
.lba        dd 0
.lba48      dd 0

; Perform an extended read using BIOS
; On error, jump to read_failed and never returns
extended_read:
    mov ah, 0x42
    mov si, DAP
    mov dl, 0x80
    int 0x13

    jc read_failed

    ret

; Loaded at 0x410:0x0 (0x4100)
second_step:
    ; Set data segment
    mov ax, 0x410
    mov ds, ax

    ; Used for disk access
    mov ax, FREE_SEGMENT
    mov gs, ax

    mov si, load_kernel
    call print_line_16

    ; 1. Read the MBR to get partition table

    mov byte [DAP.count], 1
    mov word [DAP.offset], FREE_BASE
    mov word [DAP.segment], FREE_SEGMENT
    mov dword [DAP.lba], 0

    call extended_read

    mov ax, [gs:(FREE_BASE + 446 + 8)]
    mov [partition_start], ax

    ; 2. Read the VBR of the partition to get FAT informations

    mov byte [DAP.count], 1
    mov word [DAP.offset], FREE_BASE
    mov word [DAP.segment], FREE_SEGMENT

    mov di, [partition_start]
    mov word [DAP.lba], di

    call extended_read

    mov ah, [gs:(FREE_BASE + 13)]
    mov [sectors_per_cluster], ah

    mov ax, [gs:(FREE_BASE + 14)]
    mov [reserved_sectors], ax

    mov ah, [gs:(FREE_BASE + 16)]
    mov [number_of_fat], ah

    mov ax, [gs:(FREE_BASE + 38)]
    test ax, ax
    jne sectors_per_fat_too_high

    ; sectors_per_fat (only low part)
    mov ax, [gs:(FREE_BASE + 36)]
    mov [sectors_per_fat], ax

    mov ax, [gs:(FREE_BASE + 44)]
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
    movzx ax, byte [sectors_per_cluster]
    shl ax, 4
    mov [entries_per_cluster], ax

    ; 3. Read the root directory to find the kernel executable

    mov ah, [sectors_per_cluster]
    mov byte [DAP.count], ah
    mov word [DAP.offset], FREE_BASE
    mov word [DAP.segment], FREE_SEGMENT

    ; Compute LBA from root_dir_start
    mov ax, [root_dir_start]
    sub ax, 2
    movzx bx, byte [sectors_per_cluster]
    mul bx
    mov bx, [cluster_begin]
    add ax, bx

    mov word [DAP.lba], ax

    call extended_read

    mov si, FREE_BASE
    xor cx, cx

    .next:
        mov ah, [gs:si]

        ; Test if it is the end of the directory
        test ah, ah
        je .end_of_directory

        mov ax, [gs:si]
        cmp ax, 0x4E49 ; NI
        jne .continue

        mov ax, [gs:(si+2)]
        cmp ax, 0x5449 ; TI
        jne .continue

        mov ax, [gs:(si+4)]
        cmp ax, 0x2020 ; space space
        jne .continue

        mov ax, [gs:(si+6)]
        cmp ax, 0x2020 ; space space
        jne .continue

        mov ax, [gs:(si+8)]
        cmp ax, 0x4942; IB
        jne .continue

        mov ah, [gs:(si+10)]
        cmp ah, 0x4E ; N
        jne .continue

        ; cluster high
        mov ax, [gs:(si+20)]
        mov [cluster_high], ax

        ; cluster low
        mov ax, [gs:(si+26)]
        mov [cluster_low], ax

        jmp .found

    .continue:
        add si, 32
        inc cx

        mov bx, [entries_per_cluster]
        cmp cx, bx
        jne .next

    .end_of_cluster:
        mov si, multicluster_directory
        call print_line_16

        jmp error_end

    .end_of_directory:
        mov si, kernel_not_found
        call print_line_16

        jmp error_end

    .found:

    mov si, kernel_found
    call print_line_16

    ; 4. Load the kernel into memory

    mov ax, [cluster_high]
    test ax, ax
    jne cluster_too_high

    mov ax, [cluster_low]
    mov [current_cluster], ax
    mov word [current_segment], KERNEL_BASE

.next_cluster:
    mov si, star
    call print_16

    mov di, [current_segment]
    call print_int_16

    mov si, star
    call print_16

    movzx ax, [sectors_per_cluster]
    mov word [DAP.count], ax
    mov word [DAP.offset], 0x0

    mov ax, [current_segment]
    mov [DAP.segment], ax

    ; Compute LBA from current_cluster
    mov ax, [current_cluster]
    sub ax, 2
    movzx bx, byte [sectors_per_cluster]
    mul bx
    mov bx, [cluster_begin]
    add ax, bx

    mov word [DAP.lba], ax

    mov di, ax
    call print_int_16

    mov si, star
    call print_16

    mov di, [current_cluster]
    call print_int_16

    call extended_read

    mov ax, [loaded_clusters]
    inc ax
    mov [loaded_clusters], ax

    ; Compute next cluster

    ; Compute the sector of the FAT to read
    mov ax, [current_cluster]
    shl ax, 2 ; current_cluster * 4
    shr ax, 9 ; (current_cluster * 4) / 512
    mov bx, [fat_begin]
    add ax, bx ; fat_sector

    ; Read the FAT sector
    mov word [DAP.count], 1
    mov word [DAP.offset], FREE_BASE
    mov word [DAP.segment], FREE_SEGMENT
    mov word [DAP.lba], ax

    call extended_read

    mov si, [current_cluster]
    and si, 512 - 1 ; current_cluster % 512
    shl si, 2

    ; cluster low
    mov ax, [gs:(FREE_BASE + si)]
    ; cluster high
    mov bx, [gs:(FREE_BASE + si + 2)]

    cmp bx, 0x0FFF
    jl .ok

    cmp ax, 0xFFF7
    je corrupted

    cmp ax, 0xFFF8
    jge .fully_loaded

.ok:
    test bx, bx
    jne cluster_too_high

    mov [current_cluster], ax

    movzx ax, byte [sectors_per_cluster]
    shl ax, 5
    mov bx, [current_segment]
    add ax, bx
    mov [current_segment], ax

    jmp .next_cluster

.fully_loaded:
    call new_line_16

    mov di, [loaded_clusters]
    call print_int_16

    mov si, clusters_loaded
    call print_line_16

    mov si, kernel_loaded
    call print_line_16

    call KERNEL_BASE:0x0

    jmp $

cluster_too_high:
    mov si, cluster_too_high_msg
    call print_line_16

    jmp error_end

sectors_per_fat_too_high:
    mov si, sectors_per_fat_too_high_msg
    call print_line_16

    jmp error_end

corrupted:
    mov si, corrupted_disk
    call print_line_16

    jmp error_end

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

    current_cluster dw 0
    current_segment dw 0

    loaded_clusters dw 0

; Constant Datas

    load_kernel db 'Attempt to load the kernel...', 0
    kernel_found db 'Kernel found. Starting kernel loading...', 0
    kernel_loaded db 'Kernel fully loaded', 0
    clusters_loaded db ' clusters loaded', 0
    star db '*', 0

    kernel_not_found db 'Kernel not found', 0
    corrupted_disk db 'The disk seeems to be corrupted', 0
    sectors_per_fat_too_high_msg db 'Error 1. The disk is probably too big', 0
    multicluster_directory db 'Error 2. Multicluster directory are not supported', 0
    cluster_too_high_msg db 'Error 3. Only 16bit cluster are supported', 0
    read_failed_msg db 'Read disk failed', 0
    load_failed db 'Kernel loading failed', 0

; Make it sector-aligned

    times 1536-($-$$) db 0

//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

.intel_syntax noprefix

// Define the base ISRs

.global _syscall0
.global _syscall0
.global _syscall1
.global _syscall2
.global _syscall3
.global _syscall4
.global _syscall5
.global _syscall6
.global _syscall7
.global _syscall8
.global _syscall9

_syscall0:
    cli

    push 0

    jmp syscall_common_handler

_syscall1:
    cli

    push 1

    jmp syscall_common_handler

_syscall2:
    cli

    push 2

    jmp syscall_common_handler

_syscall3:
    cli

    push 3

    jmp syscall_common_handler

_syscall4:
    cli

    push 4

    jmp syscall_common_handler

_syscall5:
    cli

    push 5

    jmp syscall_common_handler

_syscall6:
    cli

    push 6

    jmp syscall_common_handler

_syscall7:
    cli

    push 7

    jmp syscall_common_handler

_syscall8:
    cli

    push 8

    jmp syscall_common_handler

_syscall9:
    cli

    push 9

    jmp syscall_common_handler

// Common handler

syscall_common_handler:
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    xor rax, rax
    mov eax, ds
    push rax

    mov eax, 0x10
    mov ds, eax
    mov es, eax
    mov es, eax
    mov gs, eax

    call _syscall_handler

    pop rax
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12

    //Was pushed by the base handler code
    add rsp, 8

    iretq // iret will clean the other automatically pushed stuff

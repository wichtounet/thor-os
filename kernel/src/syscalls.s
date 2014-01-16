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

    push rdi
    mov rdi, 0

    jmp syscall_common_handler

_syscall1:
    cli

    push rdi
    mov rdi, 1

    jmp syscall_common_handler

_syscall2:
    cli

    push rdi
    mov rdi, 2

    jmp syscall_common_handler

_syscall3:
    cli

    push rdi
    mov rdi, 3

    jmp syscall_common_handler

_syscall4:
    cli

    push rdi
    mov rdi, 4

    jmp syscall_common_handler

_syscall5:
    cli

    push rdi
    mov rdi, 5

    jmp syscall_common_handler

_syscall6:
    cli

    push rdi
    mov rdi, 6

    jmp syscall_common_handler

_syscall7:
    cli

    push rdi
    mov rdi, 7

    jmp syscall_common_handler

_syscall8:
    cli

    push rdi
    mov rdi, 8

    jmp syscall_common_handler

_syscall9:
    cli

    push rdi
    mov rdi, 9

    jmp syscall_common_handler

// Common handler

//TODO Check if really safe to trash r12
syscall_common_handler:
    push r8
    push r9
    push r10
    push r11
    push r12
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi

    call _syscall_handler

    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8

    //Was pushed by the base handler code
    pop rdi

    iretq // iret will clean the other automatically pushed stuff

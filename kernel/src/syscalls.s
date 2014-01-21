//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

.intel_syntax noprefix

.include "src/common.s"

// Define the base ISRs

.macro create_syscall number
.global _syscall\number
_syscall\number:
    cli

    push \number

    jmp syscall_common_handler
.endm

create_syscall 0
create_syscall 1
create_syscall 2
create_syscall 3
create_syscall 4
create_syscall 5
create_syscall 6
create_syscall 7
create_syscall 8
create_syscall 9

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

    restore_kernel_segments

    call _syscall_handler

    restore_user_segments

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

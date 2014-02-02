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

    push rsp
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
    save_context

    restore_kernel_segments

    mov rdi, rsp
    call _syscall_handler

    restore_context

    //Was pushed by the base handler code
    add rsp, 8
    add rsp, 8

    iretq // iret will clean the other automatically pushed stuff

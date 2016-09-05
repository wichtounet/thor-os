//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT_1_0.txt)
//=======================================================================

.intel_syntax noprefix

.include "src/common.s"

// Define the base ISRs

.macro create_syscall number
.global _syscall\number
_syscall\number:
    //Interrupts are disabled on interrupt gate,
    //so they must reenabled again
    sti

    push rax
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
    add rsp, 16

    iretq // iret will clean the other automatically pushed stuff

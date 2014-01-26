//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

.intel_syntax noprefix

.include "src/common.s"

.macro create_irq number
.global _irq\number
_irq\number:
    cli

    push \number

    jmp irq_common_handler
.endm

create_irq 0
create_irq 1
create_irq 2
create_irq 3
create_irq 4
create_irq 5
create_irq 6
create_irq 7
create_irq 8
create_irq 9
create_irq 10
create_irq 11
create_irq 12
create_irq 13
create_irq 14
create_irq 15

// Common handler

irq_common_handler:
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

    call _irq_handler

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

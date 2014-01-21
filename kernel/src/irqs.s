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

    push rdi
    mov rdi, \number

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
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12

    restore_kernel_segments

    call _irq_handler

    restore_user_segments

    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax

    //Was pushed by the base handler code
    pop rdi

    iretq // iret will clean the other automatically pushed stuff

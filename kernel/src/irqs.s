//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT_1_0.txt)
//=======================================================================

.intel_syntax noprefix

.include "src/common.s"

.macro create_irq number
.global _irq\number
_irq\number:
    push rax
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
    save_context

    restore_kernel_segments

    mov rdi, rsp
    call _irq_handler

    restore_context

    //Was pushed by the base handler code
    add rsp, 16

    iretq // iret will clean the other automatically pushed stuff

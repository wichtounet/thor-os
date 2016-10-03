//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT_1_0.txt)
//=======================================================================

.intel_syntax noprefix

.include "src/common.s"

.macro create_irq number
.global _isr\number
_isr\number:
    push \number
    push rbp

    jmp isr_common_handler
.endm

.macro create_irq_dummy number
.global _isr\number
_isr\number:
    cli

    push 0 // Dummy error code
    push \number
    push rbp

    jmp isr_common_handler
.endm

create_irq_dummy 0
create_irq_dummy 1
create_irq_dummy 2
create_irq_dummy 3
create_irq_dummy 4
create_irq_dummy 5
create_irq_dummy 6
create_irq_dummy 7
create_irq 8
create_irq_dummy 9
create_irq 10
create_irq 11
create_irq 12
create_irq 13
create_irq 14
create_irq_dummy 15
create_irq_dummy 16
create_irq_dummy 17
create_irq_dummy 18
create_irq_dummy 19
create_irq_dummy 20
create_irq_dummy 21
create_irq_dummy 22
create_irq_dummy 23
create_irq_dummy 24
create_irq_dummy 25
create_irq_dummy 26
create_irq_dummy 27
create_irq_dummy 28
create_irq_dummy 29
create_irq_dummy 30
create_irq_dummy 31

isr_common_handler:
    //TODO Kernel segments should be restored

    call _fault_handler

    // TODO At this point, it is absolutely not safe to return since most
    // registers will get trashed the fault handler must hang

    add rsp, 16 // Cleans the pushed base pointer and error number

    iretq // iret will clean the other automatically pushed stuff

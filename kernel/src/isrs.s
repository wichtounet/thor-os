//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

.intel_syntax noprefix

// Define the base ISRs

.global _isr0
.global _isr0
.global _isr1
.global _isr2
.global _isr3
.global _isr4
.global _isr5
.global _isr6
.global _isr7
.global _isr8
.global _isr9
.global _isr10
.global _isr11
.global _isr12
.global _isr13
.global _isr14
.global _isr15
.global _isr16
.global _isr17
.global _isr18
.global _isr19
.global _isr20
.global _isr21
.global _isr22
.global _isr23
.global _isr24
.global _isr25
.global _isr26
.global _isr27
.global _isr28
.global _isr29
.global _isr30
.global _isr31

_isr0:
    cli
    push 0 // Dummy error code
    push 0

    jmp isr_common_handler

_isr1:
    cli
    push 0 // Dummy error code
    push 1

    jmp isr_common_handler

_isr2:
    cli
    push 0 // Dummy error code
    push 2

    jmp isr_common_handler

_isr3:
    cli
    push 0 // Dummy error code
    push 3

    jmp isr_common_handler

_isr4:
    cli
    push 0 // Dummy error code
    push 4

    jmp isr_common_handler

_isr5:
    cli
    push 0 // Dummy error code
    push 5

    jmp isr_common_handler

_isr6:
    cli
    push 0 // Dummy error code
    push 6

    jmp isr_common_handler

_isr7:
    cli
    push 0 // Dummy error code
    push 7

    jmp isr_common_handler

_isr8:
    cli
    push 8

    jmp isr_common_handler

_isr9:
    cli
    push 0 // Dummy error code
    push 9

    jmp isr_common_handler

_isr10:
    cli
    push 10

    jmp isr_common_handler

_isr11:
    cli
    push 11

    jmp isr_common_handler

_isr12:
    cli
    push 12

    jmp isr_common_handler

_isr13:
    cli
    push 13

    jmp isr_common_handler

_isr14:
    cli
    push 14

    jmp isr_common_handler

_isr15:
    cli
    push 0 // Dummy error code
    push 15

    jmp isr_common_handler

_isr16:
    cli
    push 0 // Dummy error code
    push 16

    jmp isr_common_handler

_isr17:
    cli
    push 0 // Dummy error code
    push 17

    jmp isr_common_handler

_isr18:
    cli
    push 0 // Dummy error code
    push 18

    jmp isr_common_handler

_isr19:
    cli
    push 0 // Dummy error code
    push 19

    jmp isr_common_handler

_isr20:
    cli
    push 0 // Dummy error code
    push 20

    jmp isr_common_handler

_isr21:
    cli
    push 0 // Dummy error code
    push 21

    jmp isr_common_handler

_isr22:
    cli
    push 0 // Dummy error code
    push 22

    jmp isr_common_handler

_isr23:
    cli
    push 0 // Dummy error code
    push 23

    jmp isr_common_handler

_isr24:
    cli
    push 0 // Dummy error code
    push 24

    jmp isr_common_handler

_isr25:
    cli
    push 0 // Dummy error code
    push 25

    jmp isr_common_handler

_isr26:
    cli
    push 0 // Dummy error code
    push 26

    jmp isr_common_handler

_isr27:
    cli
    push 0 // Dummy error code
    push 27

    jmp isr_common_handler

_isr28:
    cli
    push 0 // Dummy error code
    push 28

    jmp isr_common_handler

_isr29:
    cli
    push 0 // Dummy error code
    push 29

    jmp isr_common_handler

_isr30:
    cli
    push 0 // Dummy error code
    push 30

    jmp isr_common_handler

_isr31:
    cli
    push 0 // Dummy error code
    push 31

    jmp isr_common_handler

// Common handler

isr_common_handler:

    //mov rax, _fault_handler
    call _fault_handler

    // TODO At this point, it is absolutely not safe to return since most
    // registers will get trashed the fault handler must hang

    add rsp, 8 // Cleans the pushed error number

    iretq // iret will clean the other automatically pushed stuff

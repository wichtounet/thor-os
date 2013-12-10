//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

.intel_syntax noprefix

// Define the base ISRs

.global _irq0
.global _irq0
.global _irq1
.global _irq2
.global _irq3
.global _irq4
.global _irq5
.global _irq6
.global _irq7
.global _irq8
.global _irq9
.global _irq10
.global _irq11
.global _irq12
.global _irq13
.global _irq14
.global _irq15
.global _irq16
.global _irq17
.global _irq18
.global _irq19
.global _irq20
.global _irq21
.global _irq22
.global _irq23
.global _irq24
.global _irq25
.global _irq26
.global _irq27
.global _irq28
.global _irq29
.global _irq30
.global _irq31

_irq0:
    cli
    push 0

    jmp irq_common_handler

_irq1:
    cli
    push 1

    jmp irq_common_handler

_irq2:
    cli
    push 2

    jmp irq_common_handler

_irq3:
    cli
    push 3

    jmp irq_common_handler

_irq4:
    cli
    push 4

    jmp irq_common_handler

_irq5:
    cli
    push 5

    jmp irq_common_handler

_irq6:
    cli
    push 6

    jmp irq_common_handler

_irq7:
    cli
    push 7

    jmp irq_common_handler

_irq8:
    cli
    push 8

    jmp irq_common_handler

_irq9:
    cli
    push 9

    jmp irq_common_handler

_irq10:
    cli
    push 10

    jmp irq_common_handler

_irq11:
    cli
    push 11

    jmp irq_common_handler

_irq12:
    cli
    push 12

    jmp irq_common_handler

_irq13:
    cli
    push 13

    jmp irq_common_handler

_irq14:
    cli
    push 14

    jmp irq_common_handler

_irq15:
    cli
    push 15

    jmp irq_common_handler

// Common handler

//TODO Check if really safe to trash r12
irq_common_handler:
    //Get the error code
    pop r12

    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    //Push the error
    push r12

    mov rax, _irq_handler
    call rax

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

    add rsp, 8 // Cleans the pushed error code

    iretq // iret will clean the other automatically pushed stuff

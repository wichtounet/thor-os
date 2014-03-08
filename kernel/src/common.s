//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

.intel_syntax noprefix

.macro restore_kernel_segments
    push rax
    mov eax, 0x10
    mov ds, eax
    mov es, eax
    mov es, eax
    mov gs, eax
    pop rax
.endm

.macro restore_user_segments
    pop rax
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
.endm

.macro save_context
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
    push rax
.endm

.macro restore_context
    pop rax
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
.endm

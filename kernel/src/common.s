//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

.intel_syntax noprefix

.macro restore_kernel_segments
    xor rax, rax
    mov eax, ds
    push rax

    mov eax, 0x10
    mov ds, eax
    mov es, eax
    mov es, eax
    mov gs, eax
.endm

.macro restore_user_segments
    pop rax
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
.endm


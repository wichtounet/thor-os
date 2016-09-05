//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT_1_0.txt)
//=======================================================================

.intel_syntax noprefix

#include "include/scheduler_asm.hpp"
.include "src/common.s"

.global init_task_switch
.global task_switch

// extern void init_task_switch(size_t next);

init_task_switch:
// Switch to the new CR3
    push rdi
    call get_process_cr3
    pop rdi
    mov cr3, rax

    push rdi
    call get_context_address
    pop rdi
    mov rsp, [rax]

    restore_context_light

    //Was pushed by the base handler code
    add rsp, 8

    iretq // iret will clean the other automatically pushed stuff

// extern void task_switch(size_t current, size_t next);

task_switch:
    push rax

// Those are perhaps not necessary
    mov rax, ss
    push rax
    mov rax, rsp
    push rax

    pushfq
    mov rax, cs
    push rax
    lea rax, [resume_rip]
    push rax

// Fake error code
    push 0

// Push the current context on the stack
    save_context

// Save the new context pointer
    push rdi
    push rsi
    call get_context_address
    pop rsi
    pop rdi
    mov [rax], rsp

// Switch to the new CR3
    push rdi
    push rsi
    mov rdi, rsi
    call get_process_cr3
    pop rsi
    pop rdi
    mov cr3, rax

//Switch to the new task's stack
    push rdi
    push rsi
    mov rdi, rsi
    call get_context_address
    pop rsi
    pop rdi
    mov rsp, [rax]

    restore_context

    //Was pushed by the base handler code
    add rsp, 8

    iretq // iret will clean the other automatically pushed stuff

resume_rip:
    pop rax
    pop rax
    retq

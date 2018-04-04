//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <types.hpp>

namespace interrupt {

constexpr const size_t SYSCALL_FIRST = 50;
constexpr const size_t SYSCALL_MAX = 10;

struct fault_regs {
    uint64_t rbp;
    uint64_t error_no;
    uint64_t error_code;
    uint64_t rip;
    uint64_t rflags;
    uint64_t cs;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

struct syscall_regs {
    sse_128 xmm_registers[16];
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rbp;
    uint64_t code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ds;
} __attribute__((packed));

void setup_interrupts();

bool register_irq_handler(size_t irq, void (*handler)(syscall_regs*, void*), void* data);
bool register_syscall_handler(size_t irq, void (*handler)(syscall_regs*));

bool unregister_irq_handler(size_t irq, void (*handler)(syscall_regs*, void*));
bool unregister_syscall_handler(size_t irq, void (*handler)(syscall_regs*));

} //end of interrupt namespace

#endif

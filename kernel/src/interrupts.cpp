//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "interrupts.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "isrs.hpp"
#include "console.hpp"

namespace {

struct idt_entry {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint16_t flags;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idtr {
    uint16_t limit;
    idt_entry* base;
} __attribute__((packed));

idt_entry idt_64[64];
idtr idtr_64;

void idt_set_gate(size_t gate, void (*function)(void), uint16_t gdt_selector, uint16_t flags){
    auto& entry = idt_64[gate];

    entry.segment_selector = gdt_selector;
    entry.flags = flags;
    entry.reserved = 0;

    auto function_address = reinterpret_cast<uintptr_t>(function);
    entry.offset_low = function_address & 0xFFFF;
    entry.offset_middle = (function_address >> 16) & 0xFFFF;
    entry.offset_high= function_address  >> 32;
}

} //end of anonymous namespace

struct regs {
    uint64_t error_no;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

extern "C" {

void _fault_handler(regs* regs){
    k_printf("Exception (%d) occured\n", regs->error_no);
    //TODO Complete that
    while(true){};
}

} //end of extern "C"

void interrupt::install_idt(){
    //Set the correct values inside IDTR
    idtr_64.limit = (64 * 16) - 1;
    idtr_64.base = &idt_64[0];

    //Clear the IDT
    std::fill_n(reinterpret_cast<size_t*>(idt_64), 64 * 2, 0);

    //Give the IDTR address to the CPU
    asm volatile("lidt [%0]" : : "m" (idtr_64));
}

void interrupt::install_isrs(){
    //TODO The GDT Selector should be computed in a better way

    idt_set_gate(0, _isr0, 0x18, 0x8E);
    idt_set_gate(1, _isr1, 0x18, 0x8E);
    idt_set_gate(2, _isr2, 0x18, 0x8E);
    idt_set_gate(3, _isr3, 0x18, 0x8E);
    idt_set_gate(4, _isr4, 0x18, 0x8E);
    idt_set_gate(5, _isr5, 0x18, 0x8E);
    idt_set_gate(6, _isr6, 0x18, 0x8E);
    idt_set_gate(7, _isr7, 0x18, 0x8E);
    idt_set_gate(8, _isr8, 0x18, 0x8E);
    idt_set_gate(9, _isr9, 0x18, 0x8E);
    idt_set_gate(10, _isr10, 0x18, 0x8E);
    idt_set_gate(11, _isr11, 0x18, 0x8E);
    idt_set_gate(12, _isr12, 0x18, 0x8E);
    idt_set_gate(13, _isr13, 0x18, 0x8E);
    idt_set_gate(14, _isr14, 0x18, 0x8E);
    idt_set_gate(15, _isr15, 0x18, 0x8E);
    idt_set_gate(16, _isr16, 0x18, 0x8E);
    idt_set_gate(17, _isr17, 0x18, 0x8E);
    idt_set_gate(18, _isr18, 0x18, 0x8E);
    idt_set_gate(19, _isr19, 0x18, 0x8E);
    idt_set_gate(20, _isr20, 0x18, 0x8E);
    idt_set_gate(21, _isr21, 0x18, 0x8E);
    idt_set_gate(22, _isr22, 0x18, 0x8E);
    idt_set_gate(23, _isr23, 0x18, 0x8E);
    idt_set_gate(24, _isr24, 0x18, 0x8E);
    idt_set_gate(25, _isr25, 0x18, 0x8E);
    idt_set_gate(26, _isr26, 0x18, 0x8E);
    idt_set_gate(27, _isr27, 0x18, 0x8E);
    idt_set_gate(28, _isr28, 0x18, 0x8E);
    idt_set_gate(29, _isr29, 0x18, 0x8E);
    idt_set_gate(30, _isr30, 0x18, 0x8E);
    idt_set_gate(31, _isr31, 0x18, 0x8E);
}

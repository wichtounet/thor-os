//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "interrupts.hpp"
#include "types.hpp"

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

} //end of anonymous namespace

void interrupt::install_idt(){
    //Set the correct values inside IDTR
    idtr_64.limit = (64 * 16) - 1;
    idtr_64.base = &idt_64[0];

    //Give the IDTR address to the CPU
    asm volatile("lidt [%0]" : : "m" (idtr_64));
}

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "gdt.hpp"
#include "early_memory.hpp"

void gdt::flush_tss(){
    asm volatile("mov ax, %0; ltr ax;" : : "i" (gdt::TSS_SELECTOR + 0x3) : "rax");
}

gdt::task_state_segment_t& gdt::tss(){
    return *reinterpret_cast<task_state_segment_t*>(early::tss_address);
}

//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "gdt.hpp"
#include "early_memory.hpp"

void gdt::flush_tss(){
    asm volatile("mov ax, %0; ltr ax;" : : "i" (gdt::TSS_SELECTOR + 0x3) : "rax");
}

gdt::task_state_segment_t& gdt::tss(){
    return *reinterpret_cast<task_state_segment_t*>(early::tss_address);
}

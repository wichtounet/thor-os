//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "stl/types.hpp"

namespace interrupt {

struct fault_regs {
    uint64_t error_no;
    uint64_t error_code;
    uint64_t rip;
    uint64_t rflags;
    uint64_t cs;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

void setup_interrupts();

void register_irq_handler(size_t irq, void (*handler)());

} //end of interrupt namespace

#endif

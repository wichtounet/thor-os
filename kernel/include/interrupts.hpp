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

void install_idt();
void install_isrs();
void remap_irqs();
void install_irqs();
void enable_interrupts();

void register_irq_handler(size_t irq, void (*handler)());

} //end of interrupt namespace

#endif

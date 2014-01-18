//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ATA_H
#define ATA_H

#include "interrupts.hpp"

void system_call_entry(const interrupt::syscall_regs& regs);

void install_system_calls();

#endif

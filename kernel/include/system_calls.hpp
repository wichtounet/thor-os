//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef ATA_H
#define ATA_H

#include "interrupts.hpp"

void system_call_entry(const interrupt::syscall_regs& regs);

void install_system_calls();

#endif

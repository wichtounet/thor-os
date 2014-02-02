//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.hpp"
#include "interrupts.hpp"

namespace scheduler {

constexpr const size_t MAX_PROCESS = 128;

pid_t get_pid();
scheduler::process_t& get_process(pid_t pid);

void block_process(pid_t pid);
void unblock_process(pid_t pid);

void init();
void start();

void kill_current_process(interrupt::syscall_regs* regs);

void timer_reschedule(interrupt::syscall_regs* regs);
void reschedule(interrupt::syscall_regs* regs);

process_t& new_process();
void queue_process(pid_t p);

} //end of namespace scheduler

#endif

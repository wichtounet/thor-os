//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IRQ_WAIT_QUEUE_HPP
#define IRQ_WAIT_QUEUE_HPP

#include "scheduler.hpp"
#include "lock_guard.hpp"
#include "spinlock.hpp"

template<size_t S>
struct irq_wait_queue {
private:
    volatile size_t head = 0;
    volatile size_t tail = 0;

    scheduler::pid_t pids[S];
    
    spinlock lock;

public:
    bool empty() const {
        return tail - head == 0;
    }

    scheduler::pid_t signal(){
        auto pid = pids[head % S];
        ++head;

        scheduler::soft_unblock(pid);

        return pid;
    }

    void wait(){
        auto pid = scheduler::get_pid();

        scheduler::soft_block(pid);

        lock.acquire();

        pids[tail % S] = pid;
        ++tail;

        lock.release();
        scheduler::soft_reschedule(pid);
    }
};

#endif

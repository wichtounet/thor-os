//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <queue.hpp>
#include <lock_guard.hpp>

#include "spinlock.hpp"
#include "scheduler.hpp"

struct semaphore {
private:
    spinlock lock;
    volatile size_t value;
    std::queue<scheduler::pid_t> queue;

public:
    void init(size_t v){
        value = v;
    }

    void wait(){
        lock.acquire();

        if(!value){
            queue.push(scheduler::get_pid());
            scheduler::set_current_state(scheduler::process_state::BLOCKED);
            lock.release();
            scheduler::reschedule();
            lock.acquire();
        }

        --value;

        lock.release();
    }

    void signal(){
        std::lock_guard<spinlock> l(lock);

        ++value;

        if(!queue.empty()){
            auto pid = queue.top();
            queue.pop();

            scheduler::unblock_process(pid);
        }
    }
};

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "stl/queue.hpp"
#include "stl/lock_guard.hpp"

#include "spinlock.hpp"
#include "scheduler.hpp"

struct semaphore {
private:
    spinlock lock;
    size_t value;
    std::queue<scheduler::pid_t> queue;

public:
    void init(size_t v){
        value = v;
    }

    void wait(){
        std::lock_guard<spinlock> l(lock);

        if(value > 0){
            --value;
        } else {
            queue.push(scheduler::get_pid());
            scheduler::block_process(scheduler::get_pid());
        }
    }

    void signal(){
        std::lock_guard<spinlock> l(lock);

        if(queue.empty()){
            ++value;
        } else {
            auto pid = queue.top();
            scheduler::unblock_process(pid);

            queue.pop();

            //No need to increment value, the process won't
            //decrement it
        }
    }
};

#endif

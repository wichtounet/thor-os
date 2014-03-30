//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <lock_guard.hpp>

#include "spinlock.hpp"
#include "scheduler.hpp"
#include "sleep_queue.hpp"

struct semaphore {
private:
    spinlock lock;
    volatile size_t value;
    //std::queue<scheduler::pid_t> queue;
    sleep_queue queue;

public:
    void init(size_t v){
        value = v;
    }

    //TODO Make sure it doesn't have the lost wake up problem
    void wait(){
        lock.acquire();

        if(!value){
            lock.release();
            queue.sleep();
            lock.acquire();
        }

        --value;

        lock.release();
    }

    void signal(){
        std::lock_guard<spinlock> l(lock);

        ++value;

        if(!queue.empty()){
            queue.wake_up();
        }
    }
};

#endif

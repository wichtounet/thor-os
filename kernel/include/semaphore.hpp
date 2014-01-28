//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "spinlock.hpp"
#include "vector.hpp"

//TODO Implement a queue and use it for more fairness

struct semaphore {
private:
    spinlock lock;
    std::size_t value;
    std::vector<scheduler::pid_t> queue;

public:
    void init(std::size_t value) : value(value) {
        //Nothing else to init
    }

    void wait(){
        lock.acquire();

        if(value > 0){
            --value;
        } else {
            queue.push_back(scheduler::get_pid());
            scheduler::block_process(scheduler::get_pid());
        }

        lock.release();
    }

    void signal(){
        lock.acquire();

        if(queue.empty()){
            ++value;
        } else {
            auto pid = queue.pop_back();
            scheduler::unblock_process(pid);

            //No need to increment value, the process won't
            //decrement it
        }

        lock.release();
    }
};

#endif

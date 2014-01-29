//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SLEEP_QUEUE_H
#define SLEEP_QUEUE_H

#include "stl/queue.hpp"
#include "stl/lock_guard.hpp"

#include "spinlock.hpp"
#include "scheduler.hpp"

struct sleep_queue {
private:
    mutable spinlock lock;

    std::queue<scheduler::pid_t> queue;

public:
    bool empty() const {
        std::lock_guard<spinlock> l(lock);

        return queue.empty();
    }

    scheduler::pid_t wake_up(){
        std::lock_guard<spinlock> l(lock);

        //Get the first process
        auto pid = queue.top();

        //Indicate to the scheduler that this process will be able
        //to run
        scheduler::unblock_process(pid);

        //Remove the process from the queue
        queue.pop();

        return pid;
    }

    void sleep(){
        std::lock_guard<spinlock> l(lock);

        //Get the current process information
        auto pid = scheduler::get_pid();

        //This process will sleep
        scheduler::block_process(pid);

        //Enqueue the process in the sleep queue
        queue.push(pid);
    }
};

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SLEEP_QUEUE_H
#define SLEEP_QUEUE_H

#include <circular_buffer.hpp>
#include <lock_guard.hpp>

#include "spinlock.hpp"
#include "scheduler.hpp"
#include "logging.hpp"

struct sleep_queue {
private:
    mutable spinlock lock;
    circular_buffer<scheduler::pid_t, 16> queue;

public:
    bool empty() const {
        std::lock_guard<spinlock> l(lock);

        return queue.empty();
    }

    scheduler::pid_t top_process() const {
        std::lock_guard<spinlock> l(lock);

        return queue.top();
    }

    scheduler::pid_t wake_up(){
        std::lock_guard<spinlock> l(lock);

        // Get the first process
        auto pid = queue.top();

        // Remove it
        queue.pop();

        logging::logf(logging::log_level::TRACE, "sleep_queue: wake %d\n", pid);

        //Indicate to the scheduler that this process will be able to run
        scheduler::unblock_process(pid);

        return pid;
    }

    void sleep(){
        lock.acquire();

        //Get the current process information
        auto pid = scheduler::get_pid();

        logging::logf(logging::log_level::TRACE, "sleep_queue: wait %d\n", pid);

        //Enqueue the process in the sleep queue
        queue.push(pid);

        //This process will sleep
        scheduler::block_process_light(pid);

        lock.release();

        scheduler::reschedule();
    }
};

#endif

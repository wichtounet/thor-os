//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SLEEP_QUEUE_H
#define SLEEP_QUEUE_H

#include <lock_guard.hpp>

#include "spinlock.hpp"
#include "scheduler.hpp"

struct sleep_queue {
private:
    typedef spinlock lock_type;

    mutable lock_type lock;

    scheduler::sleep_queue_ptr* head = nullptr;
    scheduler::sleep_queue_ptr* tail = nullptr;

public:
    bool empty() const {
        std::lock_guard<lock_type> l(lock);

        return head == nullptr;
    }

    scheduler::pid_t top_process() const {
        std::lock_guard<lock_type> l(lock);

        return head->pid;
    }

    scheduler::pid_t wake_up(){
        std::lock_guard<lock_type> l(lock);

        //Remove the process from the queue
        auto queue_ptr = head;
        head = head->next;

        //Get the first process
        auto pid = queue_ptr->pid;

        //Indicate to the scheduler that this process will be able
        //to run
        scheduler::unblock_process(pid);

        return pid;
    }

    void sleep(){
        lock.acquire();

        //Get the current process information
        auto pid = scheduler::get_pid();

        auto queue_ptr = scheduler::queue_ptr(pid);
        queue_ptr->pid = pid;
        queue_ptr->next = nullptr;
        queue_ptr->prev = nullptr;

        //Enqueue the process in the sleep queue
        if(!head){
            head = queue_ptr;
        } else {
            tail->next = queue_ptr;
            tail = queue_ptr;
        }

        lock.release();

        //This process will sleep
        scheduler::block_process(pid);
    }
};

#endif

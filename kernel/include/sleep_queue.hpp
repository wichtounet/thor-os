//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SLEEP_QUEUE_H
#define SLEEP_QUEUE_H

#include "arch.hpp"
#include "scheduler.hpp"

struct sleep_queue {
private:
    scheduler::sleep_queue_ptr* head = nullptr;
    scheduler::sleep_queue_ptr* tail = nullptr;

public:
    bool empty() const {
        return head == nullptr;
    }

    scheduler::pid_t wake_up(){
        size_t rflags;
        arch::disable_hwint(rflags);
        
        //Get the first process
        auto pid = head->pid;

        //Remove the process from the queue
        head = head->next;

        //Indicate to the scheduler that this process will be able
        //to run
        scheduler::unblock_process(pid);
        
        arch::enable_hwint(rflags);

        return pid;
    }

    void sleep(){
        size_t rflags;
        arch::disable_hwint(rflags);

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
        
        //This process will sleep
        scheduler::block_process();

        arch::enable_hwint(rflags);
    }
};

#endif

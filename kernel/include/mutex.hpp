//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MUTEX_H
#define MUTEX_H

//TODO Ideally this should simply be a wrapper around a semaphore

struct mutex {
private:
    spinlock lock;
    volatile size_t value;
    circular_buffer<scheduler::pid_t, 16> queue;

public:
    void init(){
        value = 1;
    }

    void acquire(){
        std::lock_guard<spinlock> l(lock);

        if(value > 0){
            value = 0;
        } else {
            auto pid = scheduler::get_pid();
            queue.push(pid);
            scheduler::block_process(pid);
        }
    }

    void release(){
        std::lock_guard<spinlock> l(lock);

        if(queue.empty()){
            value = 1;
        } else {
            auto pid = queue.pop();
            scheduler::unblock_process(pid);

            //No need to increment value, the process won't
            //decrement it
        }
    }
};

#endif

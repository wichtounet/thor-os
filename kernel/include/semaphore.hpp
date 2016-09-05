//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <circular_buffer.hpp>
#include <lock_guard.hpp>

#include "spinlock.hpp"
#include "scheduler.hpp"

struct semaphore {
private:
    mutable spinlock lock;
    volatile size_t value;
    circular_buffer<scheduler::pid_t, 16> queue;

public:
    void init(size_t v){
        value = v;
    }

    void acquire(){
        lock.acquire();

        if(value > 0){
            --value;
            lock.release();
        } else {
            auto pid = scheduler::get_pid();
            queue.push(pid);

            scheduler::block_process_light(pid);
            lock.release();
            scheduler::reschedule();
        }
    }

    void release(){
        std::lock_guard<spinlock> l(lock);

        if(queue.empty()){
            ++value;
        } else {
            auto pid = queue.pop();
            scheduler::unblock_process(pid);

            //No need to increment value, the process won't
            //decrement it
        }
    }

    void release(size_t v){
        std::lock_guard<spinlock> l(lock);

        if(queue.empty()){
            value += v;
        } else {
            while(v && !queue.empty()){
                auto pid = queue.pop();
                scheduler::unblock_process(pid);
                --v;
            }

            if(v){
                value += v;
            }
        }
    }
};

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MUTEX_H
#define MUTEX_H

#include <circular_buffer.hpp>
#include <lock_guard.hpp>

#include "spinlock.hpp"
#include "scheduler.hpp"
#include "logging.hpp"

template<bool Debug = false>
struct mutex {
private:
    mutable spinlock lock;
    volatile size_t value;
    circular_buffer<scheduler::pid_t, 16> queue;
    const char* name;

public:
    void init(size_t v = 1){
        value = v;

        if(Debug){
            name = "";
        }
    }

    void set_name(const char* name){
        this->name = name;
    }

    void acquire(){
        lock.acquire();

        if(value > 0){
            value = 0;

            if(Debug){
                logging::logf(logging::log_level::TRACE, "%s(mutex): directly acquired (process %d)\n", name, scheduler::get_pid());
            }

            lock.release();
        } else {
            auto pid = scheduler::get_pid();
            queue.push(pid);

            if(Debug){
                logging::logf(logging::log_level::TRACE, "%s(mutex): wait %d\n", name, pid);
            }

            scheduler::block_process_light(pid);
            lock.release();
            scheduler::reschedule();
        }
    }

    void release(){
        std::lock_guard<spinlock> l(lock);

        if(queue.empty()){
            value = 1;
            if(Debug){
                logging::logf(logging::log_level::TRACE, "%s(mutex): direct release (process %d)\n", name, scheduler::get_pid());
            }
        } else {
            auto pid = queue.pop();
            scheduler::unblock_process(pid);

            if(Debug){
                logging::logf(logging::log_level::TRACE, "%s(mutex): wake %d\n", name, pid);
            }

            //No need to increment value, the process won't
            //decrement it
        }
    }
};

#endif

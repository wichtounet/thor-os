//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "arch.hpp"
#include "sleep_queue.hpp"

struct semaphore {
private:
    volatile size_t value;

    sleep_queue queue;

public:
    void init(size_t v){
        value = v;
    }

    void wait(){
        size_t rflags;
        arch::disable_hwint(rflags);

        if(!value){
            queue.sleep();
        }

        --value;

        arch::enable_hwint(rflags);
    }

    void signal(){
        size_t rflags;
        arch::disable_hwint(rflags);

        ++value;

        queue.wake_up();

        arch::enable_hwint(rflags);
    }
};

#endif

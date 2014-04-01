//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "sleep_queue.hpp"
#include "int_lock.hpp"

struct semaphore {
private:
    volatile size_t value;

    sleep_queue queue;

public:
    void init(size_t v){
        value = v;
    }

    void wait(){
        int_lock lock;

        if(!value){
            queue.sleep();
        }

        --value;
    }

    void signal(){
        int_lock lock;

        ++value;

        queue.wake_up();
    }
};

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SPINLOCK_H
#define SPINLOCK_H

struct spinlock {
private:
    volatile size_t lock = 0;

public:
    void acquire(){
        while(!__sync_bool_compare_and_swap(&lock, 0, 1));
        __sync_synchronize();
    }

    void release(){
        __sync_synchronize();
        lock = 0;
    }
};

#endif

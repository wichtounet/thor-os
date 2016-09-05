//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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
        //TODO The last synchronize is probably not necessary
    }

    void release(){
        __sync_synchronize();
        lock = 0;
    }
};

#endif

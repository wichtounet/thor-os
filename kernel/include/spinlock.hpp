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
       while(__sync_lock_test_and_set(&lock, 1)){
           while(lock){
               //Wait
           }
       } 
    }

    void release(){
        __sync_lock_release(&lock);
    }
};

#endif

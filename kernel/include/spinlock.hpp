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
    size_t lock;

public:
    void acquire(){
        asm volatile(".retry: ; lock bts [%0], 0; jc .retry" : : "m" (lock));
    }

    void release(){
        asm volatile("lock btr [%0], 0" : : "m" (lock) );
    }
};

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MUTEX_H
#define MUTEX_H

#include "semaphore.hpp"

struct mutex {
private:
    semaphore sem;

public:
    void init(){
        sem.init(1);
    }

    void acquire(){
        sem.wait();
    }

    void release(){
        sem.signal();
    }
};

#endif

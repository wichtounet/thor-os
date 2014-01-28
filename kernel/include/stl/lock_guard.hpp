//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LOCK_GUARD_HPP
#define LOCK_GUARD_HPP

template<typename Lock>
struct lock_guard {
    Lock& lock;

    lock_guard(Lock& lock) : lock(lock) {
        lock.acquire();
    }

    ~lock_guard(){
        lock.release();
    }
}

#endif

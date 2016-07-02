//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LOCK_GUARD_HPP
#define LOCK_GUARD_HPP

namespace std {

template<typename Lock>
struct lock_guard {
    Lock& lock;

    explicit lock_guard(Lock& l) : lock(l) {
        lock.acquire();
    }

    lock_guard(const lock_guard&) = delete;
    lock_guard& operator=(const lock_guard&) = delete;

    lock_guard(lock_guard&&) = delete;
    lock_guard& operator=(const lock_guard&&) = delete;

    ~lock_guard(){
        lock.release();
    }
};

} //End of namespace std

#endif

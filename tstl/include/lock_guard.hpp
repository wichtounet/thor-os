//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef LOCK_GUARD_HPP
#define LOCK_GUARD_HPP

namespace std {

template<typename Lock>
struct lock_guard {
    Lock& lock;

    explicit lock_guard(Lock& l) : lock(l) {
        lock.lock();
    }

    lock_guard(const lock_guard&) = delete;
    lock_guard& operator=(const lock_guard&) = delete;

    lock_guard(lock_guard&&) = delete;
    lock_guard& operator=(const lock_guard&&) = delete;

    ~lock_guard(){
        lock.unlock();
    }
};

} //End of namespace std

#endif

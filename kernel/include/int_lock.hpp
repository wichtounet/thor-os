//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef INT_LOCK_HPP
#define INT_LOCK_HPP

#include <types.hpp>

#include "arch.hpp"

struct int_lock {
private:
    size_t rflags;

public:
    void acquire(){
        arch::disable_hwint(rflags);
    }

    void release(){
        arch::enable_hwint(rflags);
    }
};

struct direct_int_lock {
private:
    int_lock lock;

public:
    direct_int_lock(){
        lock.acquire();
    }

    ~direct_int_lock(){
        lock.release();
    }
};

#endif

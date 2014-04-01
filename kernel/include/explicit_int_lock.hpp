//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef EXPLICIT_INT_LOCK_HPP
#define EXPLICIT_INT_LOCK_HPP

#include <types.hpp>

#include "arch.hpp"

struct explicit_int_lock {
private:
    size_t rflags;

public:
    void lock(){
        arch::disable_hwint(rflags);
    }

    void unlock(){
        arch::enable_hwint(rflags);
    }
};

#endif

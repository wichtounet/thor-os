//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_MOUNT_POINT_HPP
#define USER_MOUNT_POINT_HPP

#include <types.hpp>

struct mount_point {
    size_t offset_next;
    size_t length_mp;
    size_t length_dev;
    size_t length_type;
    char name; //First char
};

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_STATFS_INFO_HPP
#define USER_STATFS_INFO_HPP

#include <types.hpp>

struct statfs_info {
    size_t total_size;
    size_t free_size;
};

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_STAT_INFO_HPP
#define USER_STAT_INFO_HPP

#include <types.hpp>

#include "tlib/datetime.hpp"

constexpr const size_t STAT_FLAG_DIRECTORY = 0x1;
constexpr const size_t STAT_FLAG_HIDDEN = 0x2;
constexpr const size_t STAT_FLAG_SYSTEM = 0x2;

struct stat_info {
    size_t flags;
    size_t size;
    datetime created;
    datetime modified;
    datetime accessed;
};

#endif

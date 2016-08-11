//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IOCTL_CODES_H
#define IOCTL_CODE_H

#include <types.hpp>

enum class ioctl_request : size_t {
    GET_BLK_SIZE = 1
};

#endif

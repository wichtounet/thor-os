//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IO_HPP
#define IO_HPP

#include <types.hpp>
#include <expected.hpp>

#include "ioctl_codes.hpp"

int64_t ioctl(size_t device, ioctl_request request, void* data);

#endif

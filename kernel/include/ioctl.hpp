//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IOCTL_HPP
#define IOCTL_HPP

#include <types.hpp>

#include <tlib/ioctl_codes.hpp>

int64_t ioctl(size_t device_fd, io::ioctl_request request, void* data);

#endif

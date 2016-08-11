//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IOCTL_HPP
#define IOCTL_HPP

#include <types.hpp>
#include <string.hpp>

#include "ioctl_codes.hpp"

int64_t ioctl(const std::string& device, ioctl_request request, void* data);

#endif

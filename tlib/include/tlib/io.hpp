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

#include "tlib/ioctl_codes.hpp"
#include "tlib/config.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

int64_t ioctl(size_t device, tlib::ioctl_request request, void* data);

} //end of namespace tlib

#endif

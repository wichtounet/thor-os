//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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

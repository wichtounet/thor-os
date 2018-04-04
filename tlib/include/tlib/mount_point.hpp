//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef USER_MOUNT_POINT_HPP
#define USER_MOUNT_POINT_HPP

#include <types.hpp>

#include "tlib/config.hpp"

THOR_NAMESPACE(tlib, vfs) {

struct mount_point {
    size_t offset_next;
    size_t length_mp;
    size_t length_dev;
    size_t length_type;
    char name; //First char
};

} // end of namespace tlib

#endif

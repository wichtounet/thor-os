//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef USER_STATFS_INFO_HPP
#define USER_STATFS_INFO_HPP

#include <types.hpp>

#include "tlib/config.hpp"

THOR_NAMESPACE(tlib, vfs) {

struct statfs_info {
    size_t total_size;
    size_t free_size;
};

} // end of namespace tlib

#endif

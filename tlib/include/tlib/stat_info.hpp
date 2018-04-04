//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef USER_STAT_INFO_HPP
#define USER_STAT_INFO_HPP

#include <types.hpp>

#include "tlib/datetime.hpp"
#include "tlib/config.hpp"

THOR_NAMESPACE(tlib, vfs) {

using datetime = THOR_NAMESPACE_NAME(tlib, rtc)::datetime;

constexpr const size_t STAT_FLAG_DIRECTORY = 1 << 0;
constexpr const size_t STAT_FLAG_HIDDEN = 1 << 1;
constexpr const size_t STAT_FLAG_SYSTEM = 1 << 2;

struct stat_info {
    size_t flags;
    size_t size;
    datetime created;
    datetime modified;
    datetime accessed;
};

} // end of namespace tlib

#endif

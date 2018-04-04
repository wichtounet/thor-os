//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef USER_DIRECTORY_ENTRY_HPP
#define USER_DIRECTORY_ENTRY_HPP

#include <types.hpp>

#include "tlib/config.hpp"

THOR_NAMESPACE(tlib, vfs) {

struct directory_entry {
    size_t type;
    size_t offset_next;
    size_t length;
    char name; //First char
};

} // end of namespace tlib

#endif

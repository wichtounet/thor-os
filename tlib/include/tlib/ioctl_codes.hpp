//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef IOCTL_CODES_H
#define IOCTL_CODE_H

#include <types.hpp>

#include "tlib/config.hpp"

THOR_NAMESPACE(tlib, io) {

enum class ioctl_request : size_t {
    GET_BLK_SIZE = 1
};

} // end of namespace

#endif

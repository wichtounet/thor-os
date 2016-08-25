//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TLIB_NET_CONSTANTS_H
#define TLIB_NET_CONSTANTS_H

#include <types.hpp>

#include "tlib/config.hpp"

THOR_NAMESPACE(tlib, network) {

enum class socket_domain : size_t {
    AF_INET
};

enum class socket_type : size_t {
    RAW
};

enum class socket_protocol : size_t {
    ICMP
};

} // end of network namespace

#endif

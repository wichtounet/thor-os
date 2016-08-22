//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_ICMP_LAYER_H
#define NET_ICMP_LAYER_H

#include <types.hpp>

#include "network.hpp"
#include "ip_layer.hpp"

namespace network {

namespace icmp {

void ping(network::interface_descriptor& interface, network::ip::address addr);

} // end of icmp namespace

} // end of network namespace

#endif

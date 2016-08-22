//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_ICMP_LAYER_H
#define NET_ICMP_LAYER_H

#include <types.hpp>

#include "net/network.hpp"
#include "net/ip_layer.hpp"

namespace network {

namespace icmp {

struct header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t rest; ///< Depends on the type of packet type
} __attribute__((packed));

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

void ping(network::interface_descriptor& interface, network::ip::address addr);

} // end of icmp namespace

} // end of network namespace

#endif

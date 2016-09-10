//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_UDP_LAYER_H
#define NET_UDP_LAYER_H

#include <types.hpp>

#include "net/ethernet_layer.hpp"
#include "net/network.hpp"

namespace network {

namespace udp {

struct header {
    uint16_t source_port;
    uint16_t target_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

} // end of upd namespace

} // end of network namespace

#endif

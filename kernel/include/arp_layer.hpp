//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_ARP_LAYER_H
#define NET_ARP_LAYER_H

#include <types.hpp>

#include "ethernet_layer.hpp"

namespace network {

namespace arp {

struct header {
    uint16_t hw_type;
    uint16_t protocol_type;
    uint8_t hw_len;
    uint8_t protocol_len;
    uint16_t operation;
    uint16_t source_hw_addr[3];
    uint16_t source_protocol_addr[2];
    uint16_t target_hw_addr[3];
    uint16_t target_protocol_addr[2];
} __attribute__((packed));

void decode(network::ethernet::packet& packet);

} // end of arp namespace

} // end of network namespace

#endif

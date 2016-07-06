//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_ETHERNET_LAYER_H
#define NET_ETHERNET_LAYER_H

#include <types.hpp>
#include <string.hpp>

namespace network {

namespace ethernet {

enum class ether_type {
    IPV4,
    IPV6,
    ARP,
    UNKNOWN
};

struct packet {
    // Set from the beginning
    char* payload;
    size_t payload_size;

    // Set by ethernet
    ether_type type;
    size_t index;

    packet(char* payload, size_t payload_size) : payload(payload), payload_size(payload_size), index(0) {}
};

struct address {
    char mac[6];
} __attribute__((packed));

struct header {
    address target;
    address source;
    uint16_t type;
} __attribute__((packed));

static_assert(sizeof(address) == 6, "The size of a MAC address is 6 bytes");
static_assert(sizeof(header) == 14, "The size of the Ethernet header is 14 bytes");

void decode(packet& packet);

} // end of ethernet namespace

} // end of network namespace

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_ETHERNET_PACKET_H
#define NET_ETHERNET_PACKET_H

#include <types.hpp>

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

    // Set for user mode
    size_t fd;
    bool user;

    packet() : fd(0), user(false) {}
    packet(char* payload, size_t payload_size) : payload(payload), payload_size(payload_size), index(0), fd(0), user(false) {}
};

} // end of ethernet namespace

} // end of network namespace

#endif

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

struct echo_request_header {
    uint16_t identifier;
    uint16_t sequence;
} __attribute__((packed));

static_assert(sizeof(echo_request_header) == sizeof(header::rest), "Invalid size for echo request header");

enum class type : uint8_t {
    ECHO_REPLY = 0,
    UNREACHABLE = 3,
    SOURCE_QUENCH = 4,
    REDICT = 5,
    ECHO_REQUEST = 8,
    ROUTER_ADVERTISEMENT = 9,
    ROUTER_SOLICITATION = 10,
    TIME_EXCEEDED = 11,
    PARAMETER_PROBLEM = 12,
    TIMESTAMP = 13,
    TIMESTAMP_REPLY = 14,
    INFORMATION_REQUEST = 15,
    INFORMATION_REPLY = 16,
    ADDRESS_MASK_REQUEST = 17,
    ADDRESS_MASK_REPLY = 18,
    TRACEROUTE = 30
};

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

void ping(network::interface_descriptor& interface, network::ip::address addr);

} // end of icmp namespace

} // end of network namespace

#endif

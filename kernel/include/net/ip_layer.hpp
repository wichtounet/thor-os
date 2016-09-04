//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_IP_LAYER_H
#define NET_IP_LAYER_H

#include <types.hpp>

#include "net/network.hpp"
#include "tlib/net_constants.hpp"

namespace network {

namespace ip {

address ip32_to_ip(uint32_t raw);
uint32_t ip_to_ip32(address ip);

struct header {
    uint8_t version_ihl;
    uint8_t dscp_ecn;
    uint16_t total_len;
    uint16_t identification;
    uint16_t flags_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t source_ip;
    uint32_t target_ip;
} __attribute__((packed));

static_assert(sizeof(header) == 20, "The size of an IPv4 header must be 20 bytes");

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

network::ethernet::packet prepare_packet(network::interface_descriptor& interface, size_t size, address& destination, size_t protocol);
network::ethernet::packet prepare_packet(char* buffer, network::interface_descriptor& interface, size_t size, address& destination, size_t protocol);
void finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);

} // end of ip namespace

} // end of network namespace

#endif

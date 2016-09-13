//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_TCP_LAYER_H
#define NET_TCP_LAYER_H

#include <types.hpp>

#include "net/ethernet_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/network.hpp"

namespace network {

namespace tcp {

struct header {
    uint16_t source_port;
    uint16_t target_port;
    uint32_t sequence_number;
    uint32_t ack_number;
    uint16_t flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_pointer;
} __attribute__((packed));

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

std::expected<network::ethernet::packet> prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size);
std::expected<network::ethernet::packet> prepare_packet(char* buffer, network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size);
void finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);

} // end of tcp namespace

} // end of network namespace

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_DNS_LAYER_H
#define NET_DNS_LAYER_H

#include <types.hpp>

#include "net/ethernet_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/network.hpp"

namespace network {

namespace dns {

static_assert(sizeof(header) == 12, "DNS flags must be 96 bits");

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

std::expected<network::ethernet::packet> prepare_packet_query(network::interface_descriptor& interface, network::ip::address target_ip, uint16_t source_port, uint16_t identification, size_t payload_size);
std::expected<network::ethernet::packet> prepare_packet_query(char* buffer, network::interface_descriptor& interface, network::ip::address target_ip, uint16_t source_port, uint16_t identification, size_t payload_size);
void finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);

} // end of dns namespace

} // end of network namespace

#endif

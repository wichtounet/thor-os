//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_IP_LAYER_H
#define NET_IP_LAYER_H

#include <types.hpp>
#include <expected.hpp>

#include "net/network.hpp"
#include "tlib/net_constants.hpp"

namespace network {

namespace ip {

address ip32_to_ip(uint32_t raw);
uint32_t ip_to_ip32(address ip);
std::string ip_to_str(address ip);

bool same_network(address ip, address test);

static_assert(sizeof(header) == 20, "The size of an IPv4 header must be 20 bytes");

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

std::expected<network::ethernet::packet> prepare_packet(network::interface_descriptor& interface, size_t size, address& destination, size_t protocol);
std::expected<network::ethernet::packet> prepare_packet(char* buffer, network::interface_descriptor& interface, size_t size, address& destination, size_t protocol);
std::expected<void> finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);

} // end of ip namespace

} // end of network namespace

#endif

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
#include "net/socket.hpp"

namespace network {

namespace tcp {

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

std::expected<network::ethernet::packet> prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size);
std::expected<network::ethernet::packet> prepare_packet(char* buffer, network::interface_descriptor& interface, network::socket& socket, size_t payload_size);

void finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);
void finalize_packet(network::interface_descriptor& interface, network::socket& socket, network::ethernet::packet& p);

std::expected<void> connect(network::socket& socket, network::interface_descriptor& interface);
std::expected<void> disconnect(network::socket& socket, network::interface_descriptor& interface);

} // end of tcp namespace

} // end of network namespace

#endif

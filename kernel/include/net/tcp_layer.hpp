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

//TODO The parameters should be set in a descriptor
// + This should be done in the same manner as the other layers
std::expected<network::ethernet::packet> kernel_prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size);
std::expected<network::ethernet::packet> user_prepare_packet(char* buffer, network::socket& socket, const packet_descriptor* descriptor);

std::expected<void> finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);
std::expected<void> finalize_packet(network::interface_descriptor& interface, network::socket& socket, network::ethernet::packet& p);

std::expected<void> connect(network::socket& socket, network::interface_descriptor& interface, size_t local_port, size_t server_port, network::ip::address server);
std::expected<void> disconnect(network::socket& socket);

} // end of tcp namespace

} // end of network namespace

#endif

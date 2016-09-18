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

void init_layer();

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

std::expected<network::ethernet::packet> user_prepare_packet(char* buffer, network::socket& socket, const packet_descriptor* descriptor);

std::expected<void> finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);
std::expected<void> finalize_packet(network::interface_descriptor& interface, network::socket& socket, network::ethernet::packet& p);

std::expected<size_t> connect(network::socket& socket, network::interface_descriptor& interface, size_t server_port, network::ip::address server);
std::expected<void> disconnect(network::socket& socket);

} // end of tcp namespace

} // end of network namespace

#endif

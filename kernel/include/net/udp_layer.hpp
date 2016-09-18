//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_UDP_LAYER_H
#define NET_UDP_LAYER_H

#include <types.hpp>

#include "net/ethernet_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/network.hpp"

namespace network {

namespace udp {

struct header {
    uint16_t source_port;
    uint16_t target_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

std::expected<network::ethernet::packet> user_prepare_packet(char* buffer, network::socket& sock, const packet_descriptor* descriptor);
std::expected<void> finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);

std::expected<size_t> client_bind(network::socket& socket, size_t server_port, network::ip::address server);
std::expected<void> client_unbind(network::socket& socket);

} // end of upd namespace

} // end of network namespace

#endif

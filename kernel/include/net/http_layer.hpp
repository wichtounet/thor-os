//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_HTTP_LAYER_H
#define NET_HTTP_LAYER_H

#include <types.hpp>

#include "net/ethernet_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/network.hpp"

namespace network {

namespace http {

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

std::expected<network::ethernet::packet> kernel_prepare_packet(network::interface_descriptor& interface, const packet_descriptor& descriptor);
std::expected<network::ethernet::packet> user_prepare_packet(char* buffer, network::interface_descriptor& interface, const packet_descriptor* descriptor);

std::expected<void> finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);

} // end of http namespace

} // end of network namespace

#endif

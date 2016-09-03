//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_ICMP_LAYER_H
#define NET_ICMP_LAYER_H

#include <types.hpp>

#include "tlib/net_constants.hpp"

#include "net/network.hpp"
#include "net/ip_layer.hpp"

namespace network {

namespace icmp {

static_assert(sizeof(echo_request_header) == sizeof(header::rest), "Invalid size for echo request header");

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

network::ethernet::packet prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t payload_size, type t, size_t code);
void finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);

void ping(network::interface_descriptor& interface, network::ip::address addr);

} // end of icmp namespace

} // end of network namespace

#endif

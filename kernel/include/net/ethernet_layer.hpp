//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_ETHERNET_LAYER_H
#define NET_ETHERNET_LAYER_H

#include <types.hpp>

#include "net/network.hpp"
#include "net/ethernet_packet.hpp"

namespace network {

namespace ethernet {

struct address {
    char mac[6];
} __attribute__((packed));

struct header {
    address target;
    address source;
    uint16_t type;
} __attribute__((packed));

static_assert(sizeof(address) == 6, "The size of a MAC address is 6 bytes");
static_assert(sizeof(header) == 14, "The size of the Ethernet header is 14 bytes");

uint64_t mac6_to_mac64(const char* mac);
void mac64_to_mac6(uint64_t input, char* mac);

void decode(network::interface_descriptor& interface, packet& packet);

packet prepare_packet(network::interface_descriptor& interface, size_t size, size_t destination, ether_type type);
packet prepare_packet(char* buffer, network::interface_descriptor& interface, size_t size, size_t destination, ether_type type);
void finalize_packet(network::interface_descriptor& interface, packet& p);

} // end of ethernet namespace

} // end of network namespace

#endif

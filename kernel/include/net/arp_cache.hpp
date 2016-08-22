//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_ARP_CACHE_H
#define NET_ARP_CACHE_H

#include <types.hpp>

#include "net/ip_layer.hpp"
#include "net/network.hpp"

namespace network {

namespace arp {

void update_cache(uint64_t mac, network::ip::address ip);

bool is_mac_cached(uint64_t mac);
bool is_ip_cached(network::ip::address ip);

network::ip::address get_ip(uint64_t mac);
uint64_t get_mac(network::ip::address ip);

uint64_t get_mac_force(network::interface_descriptor& interface, network::ip::address ip);

} // end of arp namespace

} // end of network namespace

#endif

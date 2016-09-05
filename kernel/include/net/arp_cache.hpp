//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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

/*!
 * \brief Returns the IP address of the given MAC address.
 * The address must be in cache.
 * \param ip The MAC address to look for in the cache
 * \return The IP address of the MAC address
 */
network::ip::address get_ip(uint64_t mac);

/*!
 * \brief Returns the MAC address of the given IP address.
 * The address must be in cache.
 * \param ip The IP address to look for in the cache
 * \return The MAC address of the IP address
 */
uint64_t get_mac(network::ip::address ip);

uint64_t get_mac_force(network::interface_descriptor& interface, network::ip::address ip);

} // end of arp namespace

} // end of network namespace

#endif

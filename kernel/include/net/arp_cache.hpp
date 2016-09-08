//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_ARP_CACHE_H
#define NET_ARP_CACHE_H

#include <types.hpp>
#include <expected.hpp>

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

/*!
 * \brief Returns the MAC address of the given IP address. If the
 * address is not cached a request will be generated and it will
 * wait for the answer (may stall indefinitely).
 * \param interface The network interface to use
 * \param ip The IP address to look for in the cache
 * \return The MAC address of the IP address
 */
std::expected<uint64_t> get_mac_force(network::interface_descriptor& interface, network::ip::address ip);

/*!
 * \brief Returns the MAC address of the given IP address. If the
 * address is not cached a request will be generated and it will
 * wait for the answer or until the timeout (in ms) is reached.
 * \param interface The network interface to use
 * \param ip The IP address to look for in the cache
 * \param ms The maximum time, in milliseconds, to wait
 * \return The MAC address of the IP address
 */
std::expected<uint64_t> get_mac_force(network::interface_descriptor& interface, network::ip::address ip, size_t ms);

} // end of arp namespace

} // end of network namespace

#endif

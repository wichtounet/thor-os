//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_ARP_CACHE_H
#define NET_ARP_CACHE_H

#include <types.hpp>
#include <vector.hpp>
#include <expected.hpp>

#include "tlib/net_constants.hpp"

#include "net/ip_layer.hpp"
#include "net/interface.hpp"

namespace network {

namespace ethernet {
struct layer;
}

namespace arp {

struct layer;

/*!
 * \brief An entry in the ARP cache
 */
struct cache_entry {
    uint64_t mac;            ///< The MAC address
    network::ip::address ip; ///< The IP address

    /*!
     * \brief Construct a new empty cache entry
     */
    cache_entry(){}

    /*!
     * \brief Construct a new cache entry
     * \param mac The MAC address
     * \param ip The IP address
     */
    cache_entry(uint64_t mac, network::ip::address ip) : mac(mac), ip(ip) {}
};

/*!
 * \brief An ARP cache
 */
struct cache {
    /*!
     * \brief Construct a new cache.
     * \param layer The ARP layer
     * \param parent The parent layer (ethernet layer)
     */
    cache(network::arp::layer* layer, network::ethernet::layer* parent);

    /*!
     * \brief Update the cache entry for the given MAC address
     * \param mac The MAC address
     * \param ip The IP address
     */
    void update_cache(uint64_t mac, network::ip::address ip);

    /*!
     * \brief Indicates if the given MAC address is cached or not
     */
    bool is_mac_cached(uint64_t mac) const ;

    /*!
     * \brief Indicates if the given IP address is cached or not
     */
    bool is_ip_cached(network::ip::address ip) const ;

    /*!
     * \brief Returns the IP address of the given MAC address.
     * The address must be in cache.
     * \param ip The MAC address to look for in the cache
     * \return The IP address of the MAC address
     */
    network::ip::address get_ip(uint64_t mac) const ;

    /*!
     * \brief Returns the MAC address of the given IP address.
     * The address must be in cache.
     * \param ip The IP address to look for in the cache
     * \return The MAC address of the IP address
     */
    uint64_t get_mac(network::ip::address ip) const ;

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

private:
    std::expected<void> arp_request(network::interface_descriptor& interface, network::ip::address ip);

    network::arp::layer* arp_layer; ///< The ARP layer
    network::ethernet::layer* ethernet_layer; ///< The ethernet layer

    std::vector<cache_entry> mac_cache; ///< The cache of MAC addresses
};

} // end of arp namespace

} // end of network namespace

#endif

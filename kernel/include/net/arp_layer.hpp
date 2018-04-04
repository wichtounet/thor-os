//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_ARP_LAYER_H
#define NET_ARP_LAYER_H

#include <types.hpp>

#include "conc/condition_variable.hpp"

#include "tlib/net_constants.hpp"

#include "net/packet.hpp"
#include "net/interface.hpp"
#include "net/arp_cache.hpp"

namespace network {

namespace ethernet {
struct layer;
}

namespace arp {

/*!
 * \brief ARP packet header
 */
struct header {
    uint16_t hw_type; ///< The Hardware type
    uint16_t protocol_type; ///< The protocole type
    uint8_t hw_len; ///< The hardware address length
    uint8_t protocol_len; ///< The protocol address length
    uint16_t operation; ///< The operation code
    uint16_t source_hw_addr[3]; ///< The source hardware address
    uint16_t source_protocol_addr[2]; ///< The source protocol address
    uint16_t target_hw_addr[3]; ///< The target hardware address
    uint16_t target_protocol_addr[2]; ///< The target protocol address
} __attribute__((packed));

/*!
 * \brief Converts a 3x16 bit mac address to its single 64bit representation
 */
uint64_t mac3_to_mac64(uint16_t* source_mac);

/*!
 * \brief Converts a single 64bit MAC address to its 3x16bit representation
 * \param source_mac The source 64bit address
 * \param mac The output 3x16bit address
 */
void mac64_to_mac3(uint64_t source_mac, uint16_t* mac);

/*!
 * \brief Converts a 2x16 bit mac address to its single 32bit representation
 */
network::ip::address ip2_to_ip(uint16_t* source_ip);

/*!
 * \brief Converts a single 32bit IP address to its 2x16bit representation
 * \param source_ip The source 32bit address
 * \param ip The output 2x16bit address
 */
void ip_to_ip2(network::ip::address source_ip, uint16_t* ip);

/*!
 * \brief ARP layer implementation
 */
struct layer {
    /*!
     * \brief Constructs the layer
     * \param parent The parent layer
     */
    layer(network::ethernet::layer* parent);

    /*!
     * \brief Decode a network packet.
     *
     * This must only be called from the ethernet layer.
     *
     * \param interface The interface on which the packet was received
     * \param packet The packet to decode
     */
    void decode(network::interface_descriptor& interface, network::packet_p& packet);

    /*!
     * \brief Returns a reference to the ARP cache
     */
    network::arp::cache& get_cache();

    /*!
     * \brief Wait for an ARP packet, indefinitely.
     */
    void wait_for_reply();

    /*!
     * \brief Wait for an ARP packet, for the given time
     */
    void wait_for_reply(size_t ms);

private:
    network::ethernet::layer* parent; ///< The parent layer (ethernet)
    network::arp::cache _cache; ///< The ARP cache

    condition_variable wait_queue; ///< Wait queue for ARP packets
};

} // end of arp namespace

} // end of network namespace

#endif

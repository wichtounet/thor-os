//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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

struct header {
    uint16_t hw_type;
    uint16_t protocol_type;
    uint8_t hw_len;
    uint8_t protocol_len;
    uint16_t operation;
    uint16_t source_hw_addr[3];
    uint16_t source_protocol_addr[2];
    uint16_t target_hw_addr[3];
    uint16_t target_protocol_addr[2];
} __attribute__((packed));

uint64_t mac3_to_mac64(uint16_t* source_mac);
network::ip::address ip2_to_ip(uint16_t* source_ip);

void mac64_to_mac3(uint64_t source_mac, uint16_t* mac);
void ip_to_ip2(network::ip::address source_ip, uint16_t* ip);

struct layer {
    layer(network::ethernet::layer* parent);

    /*!
     * \brief Decode a network packet.
     *
     * This must only be called from the ethernet layer.
     *
     * \param interface The interface on which the packet was received
     * \param packet The packet to decode
     */
    void decode(network::interface_descriptor& interface, network::packet& packet);

    /*!
     * \brief Returns a reference to the ARP cache
     */
    network::arp::cache& get_cache();

    void wait_for_reply();
    void wait_for_reply(size_t ms);

private:
    network::ethernet::layer* parent; ///< The parent layer (ethernet)
    network::arp::cache _cache; ///< The ARP cache

    condition_variable wait_queue; ///< Wait queue for ARP packets
};

} // end of arp namespace

} // end of network namespace

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_ETHERNET_LAYER_H
#define NET_ETHERNET_LAYER_H

#include <types.hpp>
#include <expected.hpp>

#include "tlib/net_constants.hpp"

#include "net/interface.hpp"
#include "net/packet.hpp"

namespace network {

namespace arp {
struct layer;
}

namespace ip {
struct layer;
}

namespace ethernet {

static_assert(sizeof(address) == 6, "The size of a MAC address is 6 bytes");
static_assert(sizeof(header) == 14, "The size of the Ethernet header is 14 bytes");

/*!
 * \brief Convert a 6 sequential byte MAC address to its single 64bit representation
 */
uint64_t mac6_to_mac64(const char* mac);

/*!
 * \brief Convert a 64bit MAC to its 6 sequential byte MAC address
 * \param input The 64bit MAC input
 * \param mac The output 6 sequential byte MAC address representation
 */
void mac64_to_mac6(uint64_t input, char* mac);

/*!
 * \brief Ethernet layer implementation
 */
struct layer {
    /*!
     * \brief Decode a network packet.
     *
     * This must only be called from the network interface.
     *
     * \param interface The interface on which the packet was received
     * \param packet The packet to decode
     */
    void decode(network::interface_descriptor& interface, packet_p& packet);

    /*!
     * \brief Prepare a packet for the kernel
     * \param interface The interface on which to prepare the packet for
     * \param descriptor The packet descriptor
     * \return the prepared packet or an error
     */
    std::expected<packet_p> kernel_prepare_packet(network::interface_descriptor& interface, const packet_descriptor& descriptor);

    /*!
     * \brief Prepare a packet for the user
     * \param buffer The buffer to write the packet to
     * \param interface The interface on which to prepare the packet for
     * \param descriptor The packet descriptor
     * \return the prepared packet or an error
     */
    std::expected<packet_p> user_prepare_packet(char* buffer, network::interface_descriptor& interface, const packet_descriptor* descriptor);

    /*!
     * \brief Finalize a prepared packet
     * \param interface The interface on which to finalize the packet
     * \param p The packet to finalize
     * \return nothing or an error
     */
    std::expected<void> finalize_packet(network::interface_descriptor& interface, packet_p& p);

    /*!
     * \brief Register the ARP layer
     * \param layer The ARP layer
     */
    void register_arp_layer(network::arp::layer* layer);

    /*!
     * \brief Register the IP layer
     * \param layer The IP layer
     */
    void register_ip_layer(network::ip::layer* layer);

private:
    network::arp::layer* arp_layer; ///< The ARP layer
    network::ip::layer* ip_layer;   ///< The IP layer
};

} // end of ethernet namespace

} // end of network namespace

#endif

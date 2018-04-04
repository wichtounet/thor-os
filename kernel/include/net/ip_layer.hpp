//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_IP_LAYER_H
#define NET_IP_LAYER_H

#include <types.hpp>
#include <expected.hpp>

#include "tlib/net_constants.hpp"

#include "net/interface.hpp"
#include "net/packet.hpp"

namespace network {

namespace ethernet {
struct layer;
}

namespace arp {
struct layer;
}

namespace icmp {
struct layer;
}

namespace udp {
struct layer;
}

namespace tcp {
struct layer;
}

namespace ip {

/*!
 * \brief Convert a raw 32bit address to the abstract representation
 */
address ip32_to_ip(uint32_t raw);

/*!
 * \brief Convert the abstract IP address to a raw 32bit address
 */
uint32_t ip_to_ip32(address ip);

/*!
 * \brief Returns the string representation of the given IP
 */
std::string ip_to_str(address ip);

/*!
 * \brief Test if two IP addresses are in the same network
 */
bool same_network(address ip, address test);

static_assert(sizeof(header) == 20, "The size of an IPv4 header must be 20 bytes");

/*!
 * \brief The IP layer implementation
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
     * \brief Prepare a packet for the kernel
     * \param interface The interface on which to prepare the packet for
     * \param descriptor The packet descriptor
     * \return the prepared packet or an error
     */
    std::expected<network::packet_p> kernel_prepare_packet(network::interface_descriptor& interface, const packet_descriptor& desc);

    /*!
     * \brief Prepare a packet for the user
     * \param buffer The buffer to write the packet to
     * \param interface The interface on which to prepare the packet for
     * \param descriptor The packet descriptor
     * \return the prepared packet or an error
     */
    std::expected<network::packet_p> user_prepare_packet(char* buffer, network::interface_descriptor& interface, const packet_descriptor* desc);

    /*!
     * \brief Finalize a prepared packet
     * \param interface The interface on which to finalize the packet
     * \param p The packet to finalize
     * \return nothing or an error
     */
    std::expected<void> finalize_packet(network::interface_descriptor& interface, network::packet_p& p);

    /*!
     * \brief Register the ICMP layer
     * \param layer The ICMP layer
     */
    void register_icmp_layer(network::icmp::layer* layer);

    /*!
     * \brief Register the ARP layer
     * \param layer The ARP layer
     */
    void register_arp_layer(network::arp::layer* layer);

    /*!
     * \brief Register the UDP layer
     * \param layer The UDP layer
     */
    void register_udp_layer(network::udp::layer* layer);

    /*!
     * \brief Register the TCP layer
     * \param layer The TCP layer
     */
    void register_tcp_layer(network::tcp::layer* layer);

private:
    std::expected<uint64_t> get_target_mac(network::interface_descriptor& interface, network::ip::address target_ip);

    network::ethernet::layer* parent;  ///< The parent layer

    network::icmp::layer* icmp_layer; ///< The ICMP layer
    network::arp::layer* arp_layer; ///< The ARP layer
    network::udp::layer* udp_layer; ///< The UDP layer
    network::tcp::layer* tcp_layer; ///< The TCP layer
};

} // end of ip namespace

} // end of network namespace

#endif

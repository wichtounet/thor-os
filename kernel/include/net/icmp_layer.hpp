//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_ICMP_LAYER_H
#define NET_ICMP_LAYER_H

#include <types.hpp>
#include <expected.hpp>

#include "tlib/net_constants.hpp"

#include "net/interface.hpp"
#include "net/packet.hpp"

namespace network {

namespace ip {
struct layer;
}

namespace icmp {

static_assert(sizeof(echo_request_header) == sizeof(header::rest), "Invalid size for echo request header");

/*!
 * \brief The ICMP layer implementation
 */
struct layer {
    /*!
     * \brief Constructs the layer
     * \param parent The parent layer
     */
    layer(network::ip::layer* parent);

    /*!
     * \brief Decode a network packet.
     *
     * This must only be called from the ip layer.
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
    std::expected<network::packet_p> kernel_prepare_packet(network::interface_descriptor& interface, const packet_descriptor& descriptor);

    /*!
     * \brief Prepare a packet for the user
     * \param buffer The buffer to write the packet to
     * \param interface The interface on which to prepare the packet for
     * \param descriptor The packet descriptor
     * \return the prepared packet or an error
     */
    std::expected<network::packet_p> user_prepare_packet(char* buffer, network::socket& sock, const packet_descriptor* descriptor);

    /*!
     * \brief Finalize a prepared packet
     * \param interface The interface on which to finalize the packet
     * \param p The packet to finalize
     * \return nothing or an error
     */
    std::expected<void> finalize_packet(network::interface_descriptor& interface, network::packet_p& p);

    /*!
     * \brief Finalize a prepared packet
     * \param interface The interface on which to finalize the packet
     * \param p The packet to finalize
     * \return nothing or an error
     */
    std::expected<void> finalize_packet(network::interface_descriptor& interface, network::socket& sock, network::packet_p& p);

private:
    network::ip::layer* parent; ///< The parent layer
};

} // end of icmp namespace

} // end of network namespace

#endif

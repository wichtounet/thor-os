//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_DNS_LAYER_H
#define NET_DNS_LAYER_H

#include <types.hpp>

#include "tlib/net_constants.hpp"

#include "net/ethernet_packet.hpp"
#include "net/interface.hpp"

namespace network {

namespace udp {
struct layer;
}

namespace dns {

static_assert(sizeof(header) == 12, "DNS flags must be 96 bits");

struct layer {
    layer(network::udp::layer* parent);

    /*!
     * \brief Decode a network packet.
     *
     * This must only be called from the IP layer.
     *
     * \param interface The interface on which the packet was received
     * \param packet The packet to decode
     */
    void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

    /*!
     * \brief Prepare a packet for the user
     * \param buffer The buffer to write the packet to
     * \param interface The interface on which to prepare the packet for
     * \param descriptor The packet descriptor
     * \return the prepared packet or an error
     */
    std::expected<network::ethernet::packet> user_prepare_packet(char* buffer, network::socket& socket, const packet_descriptor* descriptor);

    /*!
     * \brief Finalize a prepared packet
     * \param interface The interface on which to finalize the packet
     * \param p The packet to finalize
     * \return nothing or an error
     */
    std::expected<void> finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);

private:
    network::udp::layer* parent;
};

} // end of dns namespace

} // end of network namespace

#endif

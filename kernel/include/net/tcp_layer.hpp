//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_TCP_LAYER_H
#define NET_TCP_LAYER_H

#include <types.hpp>

#include "net/ethernet_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/network.hpp"
#include "net/socket.hpp"

namespace network {

namespace tcp {

/*!
 * \brief Initialize the layer
 */
void init_layer();

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
std::expected<void> finalize_packet(network::interface_descriptor& interface, network::socket& socket, network::ethernet::packet& p);

std::expected<void> send(char* target_buffer, network::socket& socket, const char* buffer, size_t n);
std::expected<size_t> receive(char* buffer, network::socket& socket, size_t n);
std::expected<size_t> receive(char* buffer, network::socket& socket, size_t n, size_t ms);

std::expected<size_t> connect(network::socket& socket, network::interface_descriptor& interface, size_t server_port, network::ip::address server);
std::expected<size_t> accept(network::socket& socket);
std::expected<size_t> accept(network::socket& socket, size_t ms);
std::expected<void> server_start(network::socket& socket, size_t server_port, network::ip::address server);
std::expected<void> disconnect(network::socket& socket);

} // end of tcp namespace

} // end of network namespace

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_UDP_LAYER_H
#define NET_UDP_LAYER_H

#include <types.hpp>

#include "net/ethernet_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/network.hpp"

namespace network {

namespace udp {

struct header {
    uint16_t source_port;
    uint16_t target_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));


struct kernel_packet_descriptor {
    size_t payload_size;
    size_t source_port;
    size_t target_port;
    network::ip::address target_ip;
};

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
 * \brief Prepare a packet for the kernel
 * \param interface The interface on which to prepare the packet for
 * \param descriptor The packet descriptor
 * \return the prepared packet or an error
 */
std::expected<network::ethernet::packet> kernel_prepare_packet(network::interface_descriptor& interface, const kernel_packet_descriptor& descriptor);

/*!
 * \brief Prepare a packet for the user
 * \param buffer The buffer to write the packet to
 * \param interface The interface on which to prepare the packet for
 * \param descriptor The packet descriptor
 * \return the prepared packet or an error
 */
std::expected<network::ethernet::packet> user_prepare_packet(char* buffer, network::socket& sock, const packet_descriptor* descriptor);

/*!
 * \brief Finalize a prepared packet
 * \param interface The interface on which to finalize the packet
 * \param p The packet to finalize
 * \return nothing or an error
 */
std::expected<void> finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p);

std::expected<size_t> client_bind(network::socket& socket, size_t server_port, network::ip::address server);
std::expected<void> server_bind(network::socket& socket, size_t server_port, network::ip::address server);

std::expected<void> client_unbind(network::socket& socket);

std::expected<size_t> receive(char* buffer, network::socket& socket, size_t n);
std::expected<size_t> receive(char* buffer, network::socket& socket, size_t n, size_t ms);

std::expected<size_t> receive_from(char* buffer, network::socket& socket, size_t n, void* address);
std::expected<size_t> receive_from(char* buffer, network::socket& socket, size_t n, size_t ms, void* address);

std::expected<void> send(char* target_buffer, network::socket& socket, const char* buffer, size_t n);

std::expected<void> send_to(char* target_buffer, network::socket& socket, const char* buffer, size_t n, void* address);

} // end of upd namespace

} // end of network namespace

#endif

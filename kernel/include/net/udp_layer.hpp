//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_UDP_LAYER_H
#define NET_UDP_LAYER_H

#include <types.hpp>
#include <atomic.hpp>

#include "tlib/net_constants.hpp"

#include "net/interface.hpp"
#include "net/packet.hpp"
#include "net/connection_handler.hpp"
#include "net/socket.hpp"

namespace network {

namespace ip {
struct layer;
}

namespace dns {
struct layer;
}

namespace dhcp {
struct layer;
}

namespace udp {

/*!
 * \brief Header of an UDP packet
 */
struct header {
    uint16_t source_port; ///< The source port
    uint16_t target_port; ///< The target port
    uint16_t length;      ///< The length of the UDP payload
    uint16_t checksum;    ///< The UDP checksum
} __attribute__((packed));

/*!
 * \brief A descriptor for a packet from the kernel
 */
struct kernel_packet_descriptor {
    size_t payload_size; ///< The size of the payload
    size_t source_port;  ///< The source port
    size_t target_port;  ///< The target port
    network::ip::address target_ip; ///< The target IP
};

/*!
 * \brief An UDP connection
 */
struct udp_connection {
    size_t local_port;                   ///< The local source port
    size_t server_port;                  ///< The server port
    network::ip::address server_address; ///< The server address

    bool connected = false; ///< Indicates if the connection is connnected
    bool server    = false; ///< Indicates if the connection is in server mode

    network::socket* socket = nullptr; ///< Pointer to the user socket
};

/*!
 * \brief The UDP layer implementation
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
     * This must only be called from the IP layer.
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
    std::expected<network::packet_p> kernel_prepare_packet(network::interface_descriptor& interface, const kernel_packet_descriptor& descriptor);

    /*!
     * \brief Prepare a packet for the user
     * \param buffer The buffer to write the packet to
     * \param interface The interface on which to prepare the packet for
     * \param descriptor The packet descriptor
     * \return the prepared packet or an error
     */
    std::expected<network::packet_p> user_prepare_packet(char* buffer, network::socket& sock, const packet_descriptor* descriptor);

    /*!
     * \brief Prepare a packet for the user
     * \param buffer The buffer to write the packet to
     * \param sock The user socket
     * \param descriptor The packet descriptor
     * \param address The address to send to
     * \return the prepared packet or an error
     */
    std::expected<network::packet_p> user_prepare_packet(char* buffer, network::socket& sock, const network::udp::packet_descriptor* descriptor, network::inet_address* address);

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

    /*!
     * \brief Bind to the socket as a client
     * \param socket the User socket
     * \param server_port The server port
     * \param server The address of the server
     * \return The allocated local port or an error
     */
    std::expected<size_t> client_bind(network::socket& socket, size_t server_port, network::ip::address server);

    /*!
     * \brief Unbind from the socket as a client
     * \param socket the User socket
     * \return Nothing or an error
     */
    std::expected<void> client_unbind(network::socket& socket);

    /*!
     * \brief Bind to the socket as a server
     * \param socket the User socket
     * \param server_port The server port
     * \param server The address of the server
     * \return Nothing or an error
     */
    std::expected<void> server_bind(network::socket& socket, size_t server_port, network::ip::address server);

    /*!
     * \brief Receive a message directly
     * \þaram buffer The buffer in which to store the message
     * \param socket The user socket
     * \param n The maximum message size
     * \return The number of bytes read or an error
     */
    std::expected<size_t> receive(char* buffer, network::socket& socket, size_t n);

    /*!
     * \brief Receive a message directly
     * \þaram buffer The buffer in which to store the message
     * \param socket The user socket
     * \param n The maximum message size
     * \param ms The maximum amout of milliseconds to wait
     * \return The number of bytes read or an error
     */
    std::expected<size_t> receive(char* buffer, network::socket& socket, size_t n, size_t ms);

    /*!
     * \brief Receive a message directly and stores the address of the sender
     * \þaram buffer The buffer in which to store the message
     * \param socket The user socket
     * \param n The maximum message size
     * \param address Pointer to the address to write
     * \return The number of bytes read or an error
     */
    std::expected<size_t> receive_from(char* buffer, network::socket& socket, size_t n, void* address);

    /*!
     * \brief Receive a message directly and stores the address of the sender
     * \þaram buffer The buffer in which to store the message
     * \param socket The user socket
     * \param n The maximum message size
     * \param address Pointer to the address to write
     * \param ms The maximum amout of milliseconds to wait
     * \return The number of bytes read or an error
     */
    std::expected<size_t> receive_from(char* buffer, network::socket& socket, size_t n, size_t ms, void* address);

    /*!
     * \brief Send a message directly
     * \param target_buffer The buffer in which to write the packet
     * \þaram socket The user socket
     * \param buffer The source message
     * \param n The size of the source message
     * \return Nothing or an error
     */
    std::expected<void> send(char* target_buffer, network::socket& socket, const char* buffer, size_t n);

    /*!
     * \brief Send a message directly to the given address
     * \param target_buffer The buffer in which to write the packet
     * \þaram socket The user socket
     * \param buffer The source message
     * \param n The size of the source message
     * \param address The address descriptor pointer
     * \return Nothing or an error
     */
    std::expected<void> send_to(char* target_buffer, network::socket& socket, const char* buffer, size_t n, void* address);

    /*!
     * \brief Register the DNS layer
     * \param layer The DNS layer
     */
    void register_dns_layer(network::dns::layer* layer);

    /*!
     * \brief Register the DHCP layer
     * \param layer The DHCP layer
     */
    void register_dhcp_layer(network::dhcp::layer* layer);

private:
    network::ip::layer* parent; ///< The parent layer

    network::dns::layer* dns_layer; ///< The DNS layer
    network::dhcp::layer* dhcp_layer; ///< The DHCP layer

    std::atomic<size_t> local_port; ///< The local port allocator

    network::connection_handler<udp_connection> connections; ///< The UDP connections
};

} // end of upd namespace

} // end of network namespace

#endif

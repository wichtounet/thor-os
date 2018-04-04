//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_TCP_LAYER_H
#define NET_TCP_LAYER_H

#include <types.hpp>
#include <atomic.hpp>
#include <queue.hpp>

#include "conc/condition_variable.hpp"

#include "net/packet.hpp"
#include "net/connection_handler.hpp"
#include "net/interface.hpp"
#include "net/socket.hpp"

namespace network {

namespace ip {
struct layer;
}

namespace tcp {

/*!
 * \brief A TCP connection
 */
struct tcp_connection {
    size_t local_port  = 0;              ///< The local source port
    size_t server_port = 0;              ///< The server port
    network::ip::address server_address; ///< The server address

    std::atomic<bool> listening;            ///< Indicates if a kernel thread is listening on this connection
    condition_variable queue;               ///< The listening queue
    std::queue<network::packet_p> packets; ///< The packets for the listening queue

    bool connected = false; ///< Indicate if the connection is connnected
    bool server    = false; ///< Indicate if the connection is a server (true) or a client (false)
    bool child     = false; ///< Indicate if the connection was created from accept

    uint32_t ack_number = 0; ///< The next ack number
    uint32_t seq_number = 0; ///< The next sequence number

    uint32_t fina_ack_number = 0; ///< The next ack number (from finalize)
    uint32_t fina_seq_number = 0; ///< The next sequence number (from finalize)

    network::socket* socket = nullptr; ///< Pointer to the user socket

    tcp_connection() : listening(false) {
        //Nothing else to init
    }

    tcp_connection(const tcp_connection& rhs) = delete;
    tcp_connection& operator=(const tcp_connection& rhs) = delete;
};

/*!
 * \brief The TCP layer implementation
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
     * \brief Prepare a packet for the user
     * \param buffer The buffer to write the packet to
     * \param interface The interface on which to prepare the packet for
     * \param descriptor The packet descriptor
     * \return the prepared packet or an error
     */
    std::expected<network::packet_p> user_prepare_packet(char* buffer, network::socket& socket, const packet_descriptor* descriptor);

    /*!
     * \brief Finalize a prepared packet
     * \param interface The interface on which to finalize the packet
     * \param p The packet to finalize
     * \return nothing or an error
     */
    std::expected<void> finalize_packet(network::interface_descriptor& interface, network::socket& socket, network::packet_p& p);

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
     * \brief Connect the socket to a server, as a client
     * \param socket The user socket
     * \þaram interface The network interface
     * \param server_port The server port
     * \param server The server address
     * \return The local allocated port or an error
     */
    std::expected<size_t> connect(network::socket& socket, network::interface_descriptor& interface, size_t server_port, network::ip::address server);

    /*!
     * \brief Disconnect the socket from a server
     * \param socket The user socket
     * \return Nothing or an error
     */
    std::expected<void> disconnect(network::socket& socket);

    /*!
     * \brief Connect the socket as a server
     * \param socket The user socket
     * \param server_port The server port
     * \param server The server address
     * \return Nothing or an error
     */
    std::expected<void> server_start(network::socket& socket, size_t server_port, network::ip::address server);

    /*!
     * \brief Wait for a connection to the server
     * \þaram socket The user socket
     * \return the socket file descriptor, or an error
     */
    std::expected<size_t> accept(network::socket& socket);

    /*!
     * \brief Wait for a connection to the server
     * \þaram socket The user socket
     * \param ms The maximum time to wait, in millliseconds
     * \return the socket file descriptor, or an error
     */
    std::expected<size_t> accept(network::socket& socket, size_t ms);

private:
    std::expected<network::packet_p> kernel_prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size);
    std::expected<network::packet_p> kernel_prepare_packet(network::interface_descriptor& interface, tcp_connection& connection, size_t payload_size);
    std::expected<void> finalize_packet_direct(network::interface_descriptor& interface, network::packet_p& p);

    network::ip::layer* parent; ///< The parent layer

    std::atomic<size_t> local_port; ///< The local port allocator

    network::connection_handler<network::tcp::tcp_connection> connections; ///< The TCP connections
};

} // end of tcp namespace

} // end of network namespace

#endif

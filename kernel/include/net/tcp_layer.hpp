//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_TCP_LAYER_H
#define NET_TCP_LAYER_H

#include <types.hpp>
#include <atomic.hpp>
#include <circular_buffer.hpp>

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

struct tcp_connection {
    size_t local_port  = 0;              ///< The local source port
    size_t server_port = 0;              ///< The server port
    network::ip::address server_address; ///< The server address

    std::atomic<bool> listening;                            ///< Indicates if a kernel thread is listening on this connection
    condition_variable queue;                               ///< The listening queue
    circular_buffer<network::packet, 32> packets; ///< The packets for the listening queue

    bool connected = false; ///< Indicate if the connection is connnected
    bool server    = false; ///< Indicate if the connection is a server (true) or a client (false)

    uint32_t ack_number = 0; ///< The next ack number
    uint32_t seq_number = 0; ///< The next sequence number

    network::socket* socket = nullptr; ///< Pointer to the user socket

    tcp_connection() : listening(false) {
        //Nothing else to init
    }

    tcp_connection(const tcp_connection& rhs) = delete;
    tcp_connection& operator=(const tcp_connection& rhs) = delete;
};

struct layer {
    layer(network::ip::layer* parent);

    /*!
     * \brief Decode a network packet.
     *
     * This must only be called from the IP layer.
     *
     * \param interface The interface on which the packet was received
     * \param packet The packet to decode
     */
    void decode(network::interface_descriptor& interface, network::packet& packet);

    /*!
     * \brief Prepare a packet for the user
     * \param buffer The buffer to write the packet to
     * \param interface The interface on which to prepare the packet for
     * \param descriptor The packet descriptor
     * \return the prepared packet or an error
     */
    std::expected<network::packet> user_prepare_packet(char* buffer, network::socket& socket, const packet_descriptor* descriptor);

    /*!
     * \brief Finalize a prepared packet
     * \param interface The interface on which to finalize the packet
     * \param p The packet to finalize
     * \return nothing or an error
     */
    std::expected<void> finalize_packet(network::interface_descriptor& interface, network::socket& socket, network::packet& p);

    std::expected<void> send(char* target_buffer, network::socket& socket, const char* buffer, size_t n);
    std::expected<size_t> receive(char* buffer, network::socket& socket, size_t n);
    std::expected<size_t> receive(char* buffer, network::socket& socket, size_t n, size_t ms);

    std::expected<size_t> connect(network::socket& socket, network::interface_descriptor& interface, size_t server_port, network::ip::address server);
    std::expected<size_t> accept(network::socket& socket);
    std::expected<size_t> accept(network::socket& socket, size_t ms);
    std::expected<void> server_start(network::socket& socket, size_t server_port, network::ip::address server);
    std::expected<void> disconnect(network::socket& socket);

private:
    std::expected<network::packet> kernel_prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size);
    std::expected<network::packet> kernel_prepare_packet(network::interface_descriptor& interface, tcp_connection& connection, size_t payload_size);
    std::expected<void> finalize_packet_direct(network::interface_descriptor& interface, network::packet& p);

    network::ip::layer* parent;

    std::atomic<size_t> local_port;

    network::connection_handler<network::tcp::tcp_connection> connections;
};

} // end of tcp namespace

} // end of network namespace

#endif

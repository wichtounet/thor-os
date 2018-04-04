//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief Network support for the Thor OS
 *
 * There are two ways of using the interface:
 *  * The free functions
 *  * The socket structure
 *
 * The second interface is easier and handles more thing automatically.
 */

#ifndef TLIB_NET_H
#define TLIB_NET_H

#include <expected.hpp>

#include "tlib/net_constants.hpp"
#include "tlib/config.hpp"
#include "tlib/malloc.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

/*!
 * \brief A network packet abstraction
 *
 * The resources are automatically freed
 */
struct packet {
    size_t fd;     ///< The packet file descriptor
    char* payload; ///< The payload pointer
    size_t index;  ///< The index at which to read or write

    packet();

    packet(const packet& p) = delete;
    packet& operator=(const packet& p) = delete;

    packet(packet&& rhs);
    packet& operator=(packet&& rhs);

    ~packet();
};

/*!
 * \brief Open a socket
 * \param domain The socket domain
 * \param type The socket type
 * \param protocol The socket protocol
 * \return The socket file descriptor or an error
 */
std::expected<size_t> socket_open(socket_domain domain, socket_type type, socket_protocol protocol);

/*!
 * \brief Close a socket.
 * \param socket_fd The socket file descriptor
 */
void socket_close(size_t socket_fd);

/*!
 * \brief Prepare a packet
 * \param socket_fd The socket file descriptor
 * \param desc The packet descriptor
 * \return The prepared packet or an error
 */
std::expected<packet> prepare_packet(size_t socket_fd, void* desc);

/*!
 * \brief Finalize a packet (send it)
 * \param socket_fd The socket file descriptor
 * \param p The packet to finalize
 * \return nothing, or an error
 */
std::expected<void> finalize_packet(size_t socket_fd, const packet& p);

/*!
 * \brief Send a message through the socket
 * \param socket_fd The socket file descriptor
 * \param buffer The source buffer
 * \param n The message's size
 * \return nothing, or an error
 */
std::expected<void> send(size_t socket_fd, const char* buffer, size_t n);

/*!
 * \brief Send a message through the socket
 * \param socket_fd The socket file descriptor
 * \param buffer The source buffer
 * \param n The message's size
 * \return nothing, or an error
 */
std::expected<void> send_to(size_t socket_fd, const char* buffer, size_t n, void* address);

/*!
 * \brief Receive a message from the socket
 * \param socket_fd The socket file descriptor
 * \param buffer The source buffer
 * \return the size of the message, or an error
 */
std::expected<size_t> receive(size_t socket_fd, char* buffer, size_t n);

/*!
 * \brief Receive a message from the socket
 * \param socket_fd The socket file descriptor
 * \param buffer The source buffer
 * \return the size of the message, or an error
 */
std::expected<size_t> receive(size_t socket_fd, char* buffer, size_t n, size_t ms);

/*!
 * \brief Receive a message from the socket
 * \param socket_fd The socket file descriptor
 * \param buffer The source buffer
 * \return the size of the message, or an error
 */
std::expected<size_t> receive_from(size_t socket_fd, char* buffer, size_t n, void* address);

/*!
 * \brief Receive a message from the socket
 * \param socket_fd The socket file descriptor
 * \param buffer The source buffer
 * \return the size of the message, or an error
 */
std::expected<size_t> receive_from(size_t socket_fd, char* buffer, size_t n, size_t ms, void* address);

/*!
 * \brief Listen for messages on the socket
 * \param socket_fd The socket file descriptor
 * \param l Indicating the listening status
 * \return nothing, or an error
 */
std::expected<void> listen(size_t socket_fd, bool l);

/*!
 * \brief Bind a destination to the datagram socket
 * \param socket_fd The socket file descriptor
 * \param server The server address
 * \return the local port, or an error
 */
std::expected<size_t> client_bind(size_t socket_fd, tlib::ip::address server);

/*!
 * \brief Bind a destination to the datagram socket
 * \param socket_fd The socket file descriptor
 * \param server The server address
 * \return the local port, or an error
 */
std::expected<size_t> client_bind(size_t socket_fd, tlib::ip::address server, size_t port);

/*!
 * \brief Bind a source to the datagram socket
 * \param socket_fd The socket file descriptor
 * \param server The server address
 * \return the local port, or an error
 */
std::expected<void> server_bind(size_t socket_fd, tlib::ip::address local);

/*!
 * \brief Bind a source to the datagram socket
 * \param socket_fd The socket file descriptor
 * \param local The local address
 * \param port The listening port
 * \return the local port, or an error
 */
std::expected<void> server_bind(size_t socket_fd, tlib::ip::address local, size_t port);

/*!
 * \brief Unbind from destination from the datagram socket
 * \param socket_fd The socket file descriptor
 * \return nothing, or an error
 */
std::expected<void> client_unbind(size_t socket_fd);

/*!
 * \brief Connect to  a destination to the stream socket
 * \param socket_fd The socket file descriptor
 * \param server The server address
 * \param port The server port
 * \return the local port, or an error
 */
std::expected<size_t> connect(size_t socket_fd, tlib::ip::address server, size_t port);

/*!
 * \brief Connect to  a destination to the stream socket
 * \param socket_fd The socket file descriptor
 * \param server The server address
 * \param port The server port
 * \return the local port, or an error
 */
std::expected<void> server_start(size_t socket_fd, tlib::ip::address server, size_t port);

/*!
 * \brief Wait for a incoming connection
 * \param socket_fd The socket file descriptor
 * \return a socket of the incoming connection
 */
std::expected<size_t> accept(size_t socket_fd);

/*!
 * \brief Wait for a incoming connection
 * \param socket_fd The socket file descriptor
 * \return a socket of the incoming connection
 */
std::expected<size_t> accept(size_t socket_fd, size_t ms);

/*!
 * \brief Disconnect from destination from the datagram socket
 * \param socket_fd The socket file descriptor
 * \return nothing, or an error
 */
std::expected<void> disconnect(size_t socket_fd);

/*!
 * \brief Wait for a packet, indifinitely
 * \param socket_fd The socket file descriptor
 * \return the received packet, or an error
 */
std::expected<packet> wait_for_packet(size_t socket_fd);

/*!
 * \brief Wait for a packet, for the given time in milliseconds
 * \param socket_fd The socket file descriptor
 * \param ms The maximum time to wait
 * \return the received packet, or an error
 */
std::expected<packet> wait_for_packet(size_t socket_fd, size_t ms);

/*!
 * \brief A network socket abstraction.
 *
 * This is easier to use than the free functions for sockets.
 */
struct socket {
    socket();
    socket(socket_domain domain, socket_type type, socket_protocol protocol);

    socket(socket&& rhs);
    socket& operator=(socket&& rhs);

    /*!
     * \brief Destruct the socket and release all acquired connections
     */
    ~socket();

    /*!
     * \brief Indicates if the socket is open or not
     * \return true if the socket is open, false otherwise
     */
    bool open() const;

    /*!
     * \brief Indicates if everything is in order
     * \return true if everything is good, false otherwise
     */
    bool good() const;

    /*!
     * \brief Indicates if the socket is connected
     * \return true if the socket is connected, false otherwise
     */
    bool connected() const;

    /*!
     * \brief Indicates if the socket is bound
     * \return true if the socket is bound, false otherwise
     */
    bool bound() const;

    /*!
     * \brief Indicates if everything is in order
     * \return true if everything is good, false otherwise
     */
    operator bool();

    /*!
     * \brief Returns the error code, if any
     * \return the error code if any, 0 otherwise
     */
    size_t error() const;

    /*!
     * \brief Clear the error code
     */
    void clear();

    /*!
     * \brief Bind the socket as a client
     * \param server The IP address
     */
    void client_bind(tlib::ip::address server);

    /*!
     * \brief Bind the socket as a client
     * \param server The IP address
     */
    void client_bind(tlib::ip::address server, size_t port);

    /*!
     * \brief Bind the socket as a server
     * \param local The IP address
     */
    void server_bind(tlib::ip::address local);

    /*!
     * \brief Bind the socket as a server
     * \param local The IP address
     */
    void server_bind(tlib::ip::address local, size_t port);

    /*!
     * \brief Unbind the client socket
     */
    void client_unbind();

    /*!
     * \brief Connect to the server (stream socket)
     * \param server The IP of the server
     * \param port The port of the server
     */
    void connect(tlib::ip::address server, size_t port);

    /*!
     * \brief Start as a server (stream socket)
     * \param server The IP of the server
     * \param port The port of the server
     */
    void server_start(tlib::ip::address server, size_t port);

    /*!
     * \brief Wait for a incoming connection
     * \param socket_fd The socket file descriptor
     * \return a socket of the incoming connection
     */
    socket accept();

    /*!
     * \brief Wait for a incoming connection
     * \param socket_fd The socket file descriptor
     * \return a socket of the incoming connection
     */
    socket accept(size_t ms);

    /*!
     * \brief Disconnnect from the server (stream socket)
     */
    void disconnect();

    /*!
     * \brief Listen to the socket (start receiving packets)
     */
    void listen(bool l);

    /*!
     * \brief Prepare a packet to send
     * \param desc The descriptor of the packet
     * \return The prepared packet
     */
    packet prepare_packet(void* desc);

    /*!
     * \brief Finalize the packet (send it)
     * \param p The packet to send
     */
    void finalize_packet(const packet& p);

    /*!
     * \brief Send a message
     * \param buffer The source buffer (the contents)
     * \param n The size of the message
     */
    void send(const char* buffer, size_t n);

    /*!
     * \brief Send a message
     * \param buffer The source buffer (the contents)
     * \param n The size of the message
     */
    void send_to(const char* buffer, size_t n, void* address);

    /*!
     * \brief Wait for a message, indifinitely.
     * \param buffer The target buffer (the contents)
     * \return The size of the message
     */
    size_t receive(char* buffer, size_t n);

    /*!
     * \brief Wait for a message, indifinitely.
     * \param buffer The target buffer (the contents)
     * \return The size of the message
     */
    size_t receive(char* buffer, size_t n, size_t ms);

    /*!
     * \brief Wait for a message, indifinitely.
     * \param buffer The target buffer (the contents)
     * \return The size of the message
     */
    size_t receive_from(char* buffer, size_t n, void* address);

    /*!
     * \brief Wait for a message, indifinitely.
     * \param buffer The target buffer (the contents)
     * \return The size of the message
     */
    size_t receive_from(char* buffer, size_t n, size_t ms, void* address);

    /*!
     * \brief Wait for a packet, indefinitely.
     * \return the received packet
     */
    packet wait_for_packet();

    /*!
     * \brief Wait for a packet, for ms milliseconds.
     *
     * If a packet is not received during the given time, the error is set in socket and the received packet is a fake one.
     *
     * \return the received packet
     */
    packet wait_for_packet(size_t ms);

private:
    socket_domain domain;     ///< The socket domain
    socket_type type;         ///< The socket type
    socket_protocol protocol; ///< The socket protocol
    size_t fd;                ///< The socket file descriptor
    size_t error_code;        ///< The error code
    bool _connected;          ///< Connection flag
    bool _bound;              ///< Bind flag
};

/*!
 * \brief Switch endianness of a 16 bit value
 * \return the input value with switched endianness
 */
inline uint16_t switch_endian_16(uint16_t nb) {
    return (nb >> 8) | (nb << 8);
}

/*!
 * \brief Switch endianness of a 32 bit value
 * \return the input value with switched endianness
 */
inline uint32_t switch_endian_32(uint32_t nb) {
    return ((nb >> 24) & 0xff) |
           ((nb << 8) & 0xff0000) |
           ((nb >> 8) & 0xff00) |
           ((nb << 24) & 0xff000000);
}

} // end of namespace tlib

#endif

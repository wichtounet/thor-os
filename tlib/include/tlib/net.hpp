//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TLIB_NET_H
#define TLIB_NET_H

#include <expected.hpp>

#include "tlib/net_constants.hpp"
#include "tlib/config.hpp"
#include "tlib/malloc.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

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

std::expected<size_t> socket_open(socket_domain domain, socket_type type, socket_protocol protocol);
void socket_close(size_t socket_fd);

std::expected<packet> prepare_packet(size_t socket_fd, void* desc);
std::expected<void> finalize_packet(size_t socket_fd, const packet& p);
std::expected<void> listen(size_t socket_fd, bool l);
std::expected<size_t> client_bind(size_t socket_fd);
std::expected<size_t> connect(size_t socket_fd, tlib::ip::address server, size_t port);
std::expected<packet> wait_for_packet(size_t socket_fd);
std::expected<packet> wait_for_packet(size_t socket_fd, size_t ms);

struct socket {
    socket(socket_domain domain, socket_type type, socket_protocol protocol);
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
     */
    void client_bind();

    void connect(tlib::ip::address server, size_t port);

    void listen(bool l);

    packet prepare_packet(void* desc);
    void finalize_packet(const packet& p);
    packet wait_for_packet();
    packet wait_for_packet(size_t ms);

private:
    socket_domain domain;     ///< The socket domain
    socket_type type;         ///< The socket type
    socket_protocol protocol; ///< The socket protocol
    size_t fd;                ///< The socket file descriptor
    size_t error_code;        ///< The error code
    size_t local_port;        ///< The local port
};

inline uint16_t switch_endian_16(uint16_t nb) {
    return (nb >> 8) | (nb << 8);
}

inline uint32_t switch_endian_32(uint32_t nb) {
    return ((nb >> 24) & 0xff) |
           ((nb << 8) & 0xff0000) |
           ((nb >> 8) & 0xff00) |
           ((nb << 24) & 0xff000000);
}

} // end of namespace tlib

#endif

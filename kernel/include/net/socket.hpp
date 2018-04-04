//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <types.hpp>
#include <queue.hpp>
#include <algorithms.hpp>
#include <circular_buffer.hpp>
#include <type_traits.hpp>

#include "tlib/net_constants.hpp"

#include "conc/condition_variable.hpp"

#include "net/packet.hpp"

#include "assert.hpp"

namespace network {

/*!
 * \brief Represent a network socket
 */
struct socket {
    size_t id;                       ///< The socket file descriptor
    socket_domain domain;            ///< The socket domain
    socket_type type;                ///< The socket type
    socket_protocol protocol;        ///< The socket protocol
    size_t next_fd;                  ///< The next file descriptor
    bool listen;                     ///< Indicates if the socket is listening to packets
    void* connection_data = nullptr; ///< Optional pointer to the connection data (TCP/UDP)

    std::vector<network::packet_p> packets; ///< Packets that are prepared with their fd

    std::queue<network::packet_p> listen_packets; ///< The packets that wait to be read in listen mode
    condition_variable listen_queue;              ///< Condition variable to wait for packets

    socket() {}
    socket(size_t id, socket_domain domain, socket_type type, socket_protocol protocol, size_t next_fd, bool listen)
            : id(id), domain(domain), type(type), protocol(protocol), next_fd(next_fd), listen(listen) {}

    /*!
     * \brief Invalidate the socket
     */
    void invalidate() {
        id = 0xFFFFFFFF;
    }

    /*!
     * \brief Indicates if the socket is valid
     */
    bool is_valid() const {
        return id != 0xFFFFFFFF;
    }

    /*!
     * \brief Register a new packet into the socket
     * \return The file descriptor of the packet
     */
    size_t register_packet(network::packet_p& packet) {
        auto fd = next_fd++;

        packet->fd = fd;

        packets.push_back(packet);

        return fd;
    }

    /*!
     * \brief Indicates if the socket has a packet with the given file descriptor
     */
    bool has_packet(size_t packet_fd) {
        for (auto& packet : packets) {
            if (packet->fd == packet_fd) {
                return true;
            }
        }

        return false;
    }

    /*!
     * \brief Returns the packet with the given file descriptor
     */
    network::packet_p& get_packet(size_t fd) {
        for (auto& packet : packets) {
            if (packet->fd == fd) {
                return packet;
            }
        }

        thor_unreachable("Should not happen");
    }

    /*!
     * \brief Removes the packet with the given file descriptor
     */
    void erase_packet(size_t fd) {
        packets.erase(std::remove_if(packets.begin(), packets.end(), [fd](network::packet_p& packet) {
            return packet->fd == fd;
        }), packets.end());
    }

    /*!
     * \brief Returns the connection data of the given type.
     *
     * This simple performs a cast to the given type, it must be the
     * correct type.
     */
    template <typename T>
    T& get_connection_data() {
        thor_assert(connection_data);
        return *reinterpret_cast<T*>(connection_data);
    }

    /*!
     * \brief Returns the connection data of the given type.
     *
     * This simple performs a cast to the given type, it must be the
     * correct type.
     */
    template <typename T>
    std::add_const_t<T>& get_connection_data() const {
        thor_assert(connection_data);
        return *reinterpret_cast<std::add_const_t<T>*>(connection_data);
    }
};

} // end of network namespace

#endif

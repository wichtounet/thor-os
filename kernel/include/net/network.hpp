//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NETWORK_H
#define NETWORK_H

#include <types.hpp>
#include <string.hpp>
#include <circular_buffer.hpp>
#include <lock_guard.hpp>
#include <tuple.hpp>
#include <expected.hpp>

#include <conc/mutex.hpp>
#include <conc/semaphore.hpp>
#include <conc/deferred_unique_semaphore.hpp>

#include "tlib/net_constants.hpp"

#include "net/ethernet_packet.hpp"

namespace network {

using socket_fd_t = size_t;

/*!
 * \brief Abstraction of a network interface
 */
struct interface_descriptor {
    size_t id;                       ///< The interface ID
    bool enabled;                    ///< true if the interface is enabled
    std::string name;                ///< The name of the interface
    std::string driver;              ///< The driver of the interface
    size_t pci_device;               ///< The pci information
    size_t mac_address;              ///< The inteface MAC address
    void* driver_data = nullptr;     ///<  The driver data
    network::ip::address ip_address; ///< The interface IP address
    network::ip::address gateway;    ///< The interface IP gateway

    size_t rx_thread_pid; ///< The pid of the rx thread
    size_t tx_thread_pid; ///< The pid of the tx thread

    size_t rx_packets_counter = 0;
    size_t rx_bytes_counter   = 0;
    size_t tx_packets_counter = 0;
    size_t tx_bytes_counter   = 0;

    mutable mutex tx_lock;                    ///< Mutex protecting the queues
    mutable semaphore tx_sem;                 ///< Semaphore for transmission
    mutable deferred_unique_semaphore rx_sem; ///< Semaphore for reception

    circular_buffer<ethernet::packet, 32> rx_queue; ///< The reception queue
    circular_buffer<ethernet::packet, 32> tx_queue; ///< The transmission queue

    void (*hw_send)(interface_descriptor&, ethernet::packet& p);

    void send(ethernet::packet& p){
        std::lock_guard<mutex> l(tx_lock);
        tx_queue.push(p);
        tx_sem.unlock();
    }

    bool is_loopback() const {
        return driver == "loopback";
    }
};

/*!
 * \brief Early initialization of the network
 */
void init();

/*
 * \brief Finalization of the network initialization.
 *
 * Must be called after initialization of the scheduler.
 */
void finalize();

/*!
 * \brief Returns the number of interfaces
 */
size_t number_of_interfaces();

/*!
 * \brief Returns the interface with the given id
 * \param index The id of the interface
 */
interface_descriptor& interface(size_t index);

/*!
 * \brief Select an interface for the given IP address
 * \þaram address The destination address
 */
interface_descriptor& select_interface(network::ip::address address);

/*!
 * \brief Open a new socket
 * \param domain The socket domain
 * \param type The socket type
 * \param protocol The socket protocol
 * \return The file descriptor on success, a negative error code otherwise
 */
std::expected<socket_fd_t> open(network::socket_domain domain, network::socket_type type, network::socket_protocol protocol);

/*!
 * \brief Close the given socket file descriptor
 */
void close(size_t fd);

/*!
 * \brief Prepare a packet
 * \param socket_fd The file descriptor of the packet
 * \param desc The packet descriptor to send (depending on the protocol)
 * \þaram buffer The buffer to hold the packet payload
 * \return a tuple containing the packet file descriptor and the packet payload index
 */
std::tuple<size_t, size_t> prepare_packet(socket_fd_t socket_fd, void* desc, char* buffer);

/*!
 * \brief Finalize a packet (send it)
 * \param socket_fd The file descriptor of the packet
 * \param packet_fd The file descriptor of the packet
 * \return 0 on success and a negative error code otherwise
 */
std::expected<void> finalize_packet(socket_fd_t socket_fd, size_t packet_fd);

/*!
 * \brief Send some data (not a packet, only a payload)
 * \param socket_fd The file descriptor of the packet
 * \return 0 on success and a negative error code otherwise
 */
std::expected<void> send(socket_fd_t socket_fd, const char* buffer, size_t n, char* target_buffer);

/*!
 * \brief Receive some data (not a packet, only a payload)
 * \param socket_fd The file descriptor of the packet
 * \return the size of the message on success and a negative error code otherwise
 */
std::expected<size_t> receive(socket_fd_t socket_fd, char* buffer, size_t n);

/*!
 * \brief Receive some data (not a packet, only a payload)
 * \param socket_fd The file descriptor of the packet
 * \return the size of the message on success and a negative error code otherwise
 */
std::expected<size_t> receive(socket_fd_t socket_fd, char* buffer, size_t n, size_t ms);

/*!
 * \brief Listen to a socket or not
 * \param socket_fd The file descriptor of the packet
 * \param listen Indicates if listen or not
 * \return 0 on success and a negative error code otherwise
 */
std::expected<void> listen(socket_fd_t socket_fd, bool listen);

/*!
 * \brief Bind a socket datagram as a client (bind a local random port)
 * \param socket_fd The file descriptor of the packet
 * \return the allocated port on success and a negative error code otherwise
 */
std::expected<size_t> client_bind(socket_fd_t socket_fd, network::ip::address address);

/*!
 * \brief Bind a socket datagram as a client (bind a local random port)
 * \param socket_fd The file descriptor of the packet
 * \return the allocated port on success and a negative error code otherwise
 */
std::expected<size_t> client_bind(socket_fd_t socket_fd, network::ip::address address, size_t port);

/*!
 * \brief Bind a socket datagram as a server
 * \param socket_fd The file descriptor of the packet
 * \return noting or an error code otherwise
 */
std::expected<void> server_bind(socket_fd_t socket_fd, network::ip::address address);

/*!
 * \brief Bind a socket datagram as a server
 * \param socket_fd The file descriptor of the packet
 * \return noting or an error code otherwise
 */
std::expected<void> server_bind(socket_fd_t socket_fd, network::ip::address address, size_t port);

/*!
 * \brief Unbind a socket datagram as a client
 * \param socket_fd The file descriptor of the packet
 * \return the allocated port on success and a negative error code otherwise
 */
std::expected<void> client_unbind(socket_fd_t socket_fd);

/*!
 * \brief Bind a socket stream as a client (bind a local random port)
 * \param socket_fd The file descriptor of the packet
 * \param server The ip address of the server
 * \param port The port of the server
 * \return the allocated port on success and a negative error code otherwise
 */
std::expected<size_t> connect(socket_fd_t socket_fd, network::ip::address address, size_t port);

/*!
 * \brief Disconnect from  a socket stream
 * \param socket_fd The file descriptor of the packet
 * \return the allocated port on success and a negative error code otherwise
 */
std::expected<void> disconnect(socket_fd_t socket_fd);

/*!
 * \brief Wait for a packet
 * \param socket_fd The file descriptor of the packet
 * \return the packet index
 */
std::expected<size_t> wait_for_packet(char* buffer, socket_fd_t socket_fd);

/*!
 * \brief Wait for a packet, for some time
 * \param socket_fd The file descriptor of the packet
 * \param ms The maximum time, in milliseconds, to wait for a packet
 * \return the packet index
 */
std::expected<size_t> wait_for_packet(char* buffer, socket_fd_t socket_fd, size_t ms);

/*!
 * \brief Propagate a packet through the raw sockets.
 *
 * This must only be called from non-dgram, non-stream network layers.
 *
 * \param packet The packet to propagate
 * \param protocol The destination protocol
 */
void propagate_packet(const ethernet::packet& packet, socket_protocol protocol);

} // end of network namespace

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NETWORK_H
#define NETWORK_H

#include <types.hpp>
#include <string.hpp>
#include <circular_buffer.hpp>
#include <mutex.hpp>
#include <semaphore.hpp>
#include <lock_guard.hpp>
#include <tuple.hpp>

#include "tlib/net_constants.hpp"

#include "net/ethernet_packet.hpp"

namespace network {

struct interface_descriptor {
    size_t id;
    bool enabled;
    std::string name;
    std::string driver;
    size_t pci_device;
    size_t mac_address;
    void* driver_data;
    network::ip::address ip_address;

    mutable mutex<> tx_lock; //To synchronize the queue
    mutable semaphore tx_sem;
    mutable semaphore rx_sem;

    size_t rx_thread_pid;
    size_t tx_thread_pid;

    circular_buffer<ethernet::packet, 32> rx_queue;
    circular_buffer<ethernet::packet, 32> tx_queue;

    void (*hw_send)(const interface_descriptor&, ethernet::packet& p);

    void send(ethernet::packet& p){
        std::lock_guard<mutex<>> l(tx_lock);
        tx_queue.push(p);
        tx_sem.release();
    }
};

void init();        // Called early on
void finalize();    // Called after scheduler is initialized

size_t number_of_interfaces();

interface_descriptor& interface(size_t index);

/*!
 * \brief Open a new socket
 * \param domain The socket domain
 * \param type The socket type
 * \param protocol The socket protocol
 * \return The file descriptor on success, a negative error code otherwise
 */
int64_t open(network::socket_domain domain, network::socket_type type, network::socket_protocol protocol);

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
std::tuple<size_t, size_t> prepare_packet(size_t socket_fd, void* desc, char* buffer);

/*!
 * \brief Finalize a packet (send it)
 * \param socket_fd The file descriptor of the packet
 * \param packet_fd The file descriptor of the packet
 * \return 0 on success and a negative error code otherwise
 */
int64_t finalize_packet(size_t socket_fd, size_t packet_fd);

/*!
 * \brief Listen to a socket or not
 * \param socket_fd The file descriptor of the packet
 * \param listen Indicates if listen or not
 * \return 0 on success and a negative error code otherwise
 */
int64_t listen(size_t socket_fd, bool listen);

/*!
 * \brief Wait for a packet
 * \param socket_fd The file descriptor of the packet
 * \return a tuple containing the packet index and payload
 */
std::tuple<int64_t, char*> wait_for_packet(size_t socket_fd);

} // end of network namespace

#endif

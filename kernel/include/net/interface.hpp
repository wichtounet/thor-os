//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include <types.hpp>
#include <string.hpp>
#include <lock_guard.hpp>
#include <shared_ptr.hpp>
#include <queue.hpp>

#include "conc/mutex.hpp"
#include "conc/semaphore.hpp"
#include "conc/deferred_unique_semaphore.hpp"

#include "net/packet.hpp"

#include "tlib/net_constants.hpp"

namespace network {

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

    size_t rx_packets_counter = 0; ///< Counter of received packets
    size_t rx_bytes_counter   = 0; ///< Counter of received bytes
    size_t tx_packets_counter = 0; ///< Counter of transmitted packets
    size_t tx_bytes_counter   = 0; ///< Counter of transmitted bytes

    mutable mutex tx_lock;                    ///< Mutex protecting the queues
    mutable semaphore tx_sem;                 ///< Semaphore for transmission
    mutable deferred_unique_semaphore rx_sem; ///< Semaphore for reception

    std::queue<network::packet_p> rx_queue;
    std::queue<network::packet_p> tx_queue;

    void (*hw_send)(interface_descriptor&, packet_p& p); ///< Driver hardware send function

    /*!
     * \brief Send a packet through this interface
     */
    void send(packet_p& p){
        std::lock_guard<mutex> l(tx_lock);
        tx_queue.push(p);
        tx_sem.unlock();
    }

    /*!
     * \brief Indicates if this function is a loopback function
     */
    bool is_loopback() const {
        return driver == "loopback";
    }
};

} // end of network namespace

#endif

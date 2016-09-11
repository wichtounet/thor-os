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

#include "ethernet_packet.hpp"

namespace network {

struct interface_descriptor {
    size_t id;
    bool enabled;
    std::string name;
    std::string driver;
    size_t pci_device;
    size_t mac_address;
    void* driver_data;

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

} // end of network namespace

#endif

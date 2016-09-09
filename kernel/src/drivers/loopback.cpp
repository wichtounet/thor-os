//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "drivers/loopback.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"
#include "physical_allocator.hpp"
#include "virtual_allocator.hpp"
#include "interrupts.hpp"
#include "paging.hpp"
#include "semaphore.hpp"
#include "paging.hpp"
#include "int_lock.hpp"

#include "net/ethernet_layer.hpp"

namespace {

struct loopback_t {
    network::interface_descriptor* interface;
};

void send_packet(network::interface_descriptor& interface, network::ethernet::packet& packet){
    logging::logf(logging::log_level::TRACE, "loopback: Transmit packet\n");

    auto packet_buffer = new char[packet.payload_size];

    std::copy_n(packet.payload, packet.payload_size, packet_buffer);

    direct_int_lock lock;

    interface.rx_queue.emplace_push(packet_buffer, packet.payload_size);
    interface.rx_sem.release();

    logging::logf(logging::log_level::TRACE, "loopback: Packet transmitted correctly\n");
}

} //end of anonymous namespace

void loopback::init_driver(network::interface_descriptor& interface){
    logging::logf(logging::log_level::TRACE, "loopback: Initialize loopback driver\n");

    loopback_t* desc = new loopback_t();
    desc->interface = &interface;

    interface.driver_data = desc;
    interface.hw_send = send_packet;

    interface.ip_address = network::ip::make_address(127, 0, 0, 1);
    //TODO interface.mac_address = mac;
}

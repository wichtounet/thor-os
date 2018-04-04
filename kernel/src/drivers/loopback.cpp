//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "drivers/loopback.hpp"

#include "net/ethernet_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"
#include "physical_allocator.hpp"
#include "virtual_allocator.hpp"
#include "interrupts.hpp"
#include "paging.hpp"

namespace {

struct loopback_t {
    network::interface_descriptor* interface;
};

void send_packet(network::interface_descriptor& interface, network::packet_p& packet){
    logging::logf(logging::log_level::TRACE, "loopback: Transmit packet\n");

    {
        direct_int_lock lock;

        interface.rx_queue.push(packet);
        interface.rx_sem.notify();
    }

    logging::logf(logging::log_level::TRACE, "loopback: Packet transmitted correctly\n");
}

} //end of anonymous namespace

void loopback::init_driver(network::interface_descriptor& interface){
    logging::logf(logging::log_level::TRACE, "loopback: Initialize loopback driver\n");

    interface.driver_data = new loopback_t();
    interface.hw_send = send_packet;

    interface.ip_address = network::ip::make_address(127, 0, 0, 1);
    //TODO maybe set a MAC address
}

void loopback::finalize_driver(network::interface_descriptor& interface){
    auto* desc = static_cast<loopback_t*>(interface.driver_data);
    desc->interface = &interface;
}

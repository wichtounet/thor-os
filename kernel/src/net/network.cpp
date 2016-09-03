//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "net/network.hpp"
#include "net/ethernet_layer.hpp"

#include "drivers/rtl8139.hpp"
#include "drivers/pci.hpp"

#include "physical_allocator.hpp"
#include "scheduler.hpp"
#include "logging.hpp"

#include "fs/sysfs.hpp"

#include "tlib/errors.hpp"

#include "net/icmp_layer.hpp"

namespace {
std::vector<network::interface_descriptor> interfaces;

void rx_thread(void* data){
    auto& interface = *reinterpret_cast<network::interface_descriptor*>(data);

    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "network: RX Thread for interface %u started (pid:%u)\n", interface.id, pid);

    while(true){
        interface.rx_sem.acquire();

        auto packet = interface.rx_queue.pop();
        network::ethernet::decode(interface, packet);
        delete[] packet.payload;
    }
}

void tx_thread(void* data){
    auto& interface = *reinterpret_cast<network::interface_descriptor*>(data);

    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "network: TX Thread for interface %u started (pid:%u)\n", interface.id, pid);

    while(true){
        interface.tx_sem.acquire();

        auto packet = interface.tx_queue.pop();
        interface.hw_send(interface, packet);
        delete[] packet.payload;
    }
}

} //end of anonymous namespace

void network::init(){
    size_t index = 0;

    for(size_t i = 0; i < pci::number_of_devices(); ++i){
        auto& pci_device = pci::device(i);

        if(pci_device.class_type == pci::device_class_type::NETWORK){
            interfaces.emplace_back();

            auto& interface = interfaces.back();

            interface.id = interfaces.size() - 1;
            interface.name = std::string("net") + std::to_string(index);
            interface.pci_device = i;
            interface.enabled = false;
            interface.driver = "";
            interface.driver_data = nullptr;
            interface.tx_lock.init(1);
            interface.tx_sem.init(0);
            interface.rx_sem.init(0);

            if(pci_device.vendor_id == 0x10EC && pci_device.device_id == 0x8139){
                interface.enabled = true;
                interface.driver = "rtl8139";

                rtl8139::init_driver(interface, pci_device);
            }

            if(interface.enabled){
                //TODO This should be configurable
                interface.ip_address = network::ip::make_address(10, 0, 2, 15);
            }

            auto p = path("/net") / interface.name;

            sysfs::set_constant_value(path("/sys"), p / "name", interface.name);
            sysfs::set_constant_value(path("/sys"), p / "driver", interface.driver);
            sysfs::set_constant_value(path("/sys"), p / "enabled", interface.enabled ? "true" : "false");
            sysfs::set_constant_value(path("/sys"), p / "pci_device", std::to_string(i));
            sysfs::set_constant_value(path("/sys"), p / "mac", std::to_string(interface.mac_address));

            ++index;
        }
    }
}

void network::finalize(){
    for(auto& interface : interfaces){
        // if the interface has a driver
        if(interface.enabled){
            auto* rx_user_stack = new char[scheduler::user_stack_size];
            auto* rx_kernel_stack = new char[scheduler::kernel_stack_size];

            auto* tx_user_stack = new char[scheduler::user_stack_size];
            auto* tx_kernel_stack = new char[scheduler::kernel_stack_size];

            auto& rx_process = scheduler::create_kernel_task_args("net_rx", rx_user_stack, rx_kernel_stack, &rx_thread, &interface);
            auto& tx_process = scheduler::create_kernel_task_args("net_tx", tx_user_stack, tx_kernel_stack, &tx_thread, &interface);

            rx_process.ppid = 1;
            tx_process.ppid = 1;

            rx_process.priority = scheduler::DEFAULT_PRIORITY;
            tx_process.priority = scheduler::DEFAULT_PRIORITY;

            scheduler::queue_system_process(rx_process.pid);
            scheduler::queue_system_process(tx_process.pid);

            interface.tx_thread_pid = tx_process.pid;
            interface.rx_thread_pid = rx_process.pid;
        }
    }
}

size_t network::number_of_interfaces(){
    return interfaces.size();
}

network::interface_descriptor& network::interface(size_t index){
    return interfaces[index];
}

int64_t network::open(network::socket_domain domain, network::socket_type type, network::socket_protocol protocol){
    if(domain != socket_domain::AF_INET){
        return -std::ERROR_SOCKET_INVALID_DOMAIN;
    }

    if(type != socket_type::RAW){
        return -std::ERROR_SOCKET_INVALID_TYPE;
    }

    if(protocol != socket_protocol::ICMP){
        return -std::ERROR_SOCKET_INVALID_PROTOCOL;
    }

    return scheduler::register_new_socket(domain, type, protocol);
}

void network::close(size_t fd){
    if(scheduler::has_socket(fd)){
        scheduler::release_socket(fd);
    }
}

std::tuple<size_t, size_t> network::prepare_packet(size_t socket_fd, void* desc, char* buffer){
    if(!scheduler::has_socket(socket_fd)){
        return {-std::ERROR_SOCKET_INVALID_FD, 0};
    }

    // TODO A socket should be bound to an interface somehow
    if(!network::number_of_interfaces()){
        return {-std::ERROR_SOCKET_NO_INTERFACE, 0};
    }

    auto& interface = network::interface(0);
    auto& socket = scheduler::get_socket(socket_fd);

    switch(socket.protocol){
        case network::socket_protocol::ICMP:
            auto descriptor = static_cast<network::icmp::packet_descriptor*>(desc);
            auto packet = network::icmp::prepare_packet(interface, descriptor->target_ip, descriptor->payload_size, descriptor->type, descriptor->code);
            auto fd = socket.register_packet(packet);

            return {fd, packet.index};
    }

    return {-std::ERROR_SOCKET_UNIMPLEMENTED, 0};
}

int64_t network::finalize_packet(size_t socket_fd, size_t packet_fd){
    if(!scheduler::has_socket(socket_fd)){
        return -std::ERROR_SOCKET_INVALID_FD;
    }

    if(!scheduler::has_socket(packet_fd)){
        return -std::ERROR_SOCKET_INVALID_PACKET_FD;
    }

    auto& interface = network::interface(0);
    auto& socket = scheduler::get_socket(socket_fd);
    auto& packet = socket.get_packet(packet_fd);

    switch(socket.protocol){
        case network::socket_protocol::ICMP:
            network::icmp::finalize_packet(interface, packet);
            socket.erase_packet(packet_fd);

            return 0;
    }

    return -std::ERROR_SOCKET_UNIMPLEMENTED, 0;
}

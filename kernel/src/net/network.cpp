//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "net/network.hpp"
#include "net/ethernet_layer.hpp"

#include "drivers/rtl8139.hpp"
#include "drivers/pci.hpp"
#include "drivers/loopback.hpp"

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

        // The memory of the packet was allocated by the interface itself, can be safely removed
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

        thor_assert(!packet.user);

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

    // Install the loopback device

    interfaces.emplace_back();

    auto& interface = interfaces.back();

    interface.id = interfaces.size() - 1;
    interface.name = "loopback";
    interface.pci_device = 0;
    interface.enabled = true;
    interface.driver = "loopback";
    interface.driver_data = nullptr;
    interface.tx_lock.init(1);
    interface.tx_sem.init(0);
    interface.rx_sem.init(0);

    loopback::init_driver(interface);
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

std::expected<network::socket_fd_t> network::open(network::socket_domain domain, network::socket_type type, network::socket_protocol protocol){
    if(domain != socket_domain::AF_INET){
        return std::make_expected_from_error<network::socket_fd_t>(std::ERROR_SOCKET_INVALID_DOMAIN);
    }

    if(type != socket_type::RAW){
        return std::make_expected_from_error<network::socket_fd_t>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    if(protocol != socket_protocol::ICMP){
        return std::make_expected_from_error<network::socket_fd_t>(std::ERROR_SOCKET_INVALID_PROTOCOL);
    }

    return scheduler::register_new_socket(domain, type, protocol);
}

void network::close(size_t fd){
    if(scheduler::has_socket(fd)){
        scheduler::release_socket(fd);
    }
}

std::tuple<size_t, size_t> network::prepare_packet(socket_fd_t socket_fd, void* desc, char* buffer){
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
            auto packet = network::icmp::prepare_packet(buffer, interface, descriptor->target_ip, descriptor->payload_size, descriptor->type, descriptor->code);

            if(packet){
                auto fd = socket.register_packet(*packet);

                return {fd, packet->index};
            } else {
                return {-packet.error(), 0};
            }
    }

    return {-std::ERROR_SOCKET_UNIMPLEMENTED, 0};
}

std::expected<void> network::finalize_packet(socket_fd_t socket_fd, size_t packet_fd){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& interface = network::interface(0);
    auto& socket = scheduler::get_socket(socket_fd);

    if(!socket.has_packet(packet_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_PACKET_FD);
    }

    auto& packet = socket.get_packet(packet_fd);

    switch(socket.protocol){
        case network::socket_protocol::ICMP:
            network::icmp::finalize_packet(interface, packet);
            socket.erase_packet(packet_fd);

            return std::make_expected();
    }

    return std::make_unexpected<void>(std::ERROR_SOCKET_UNIMPLEMENTED);
}

std::expected<void> network::listen(socket_fd_t socket_fd, bool listen){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    socket.listen = listen;

    return std::make_expected();
}

std::expected<size_t> network::wait_for_packet(char* buffer, socket_fd_t socket_fd){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(!socket.listen){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_LISTEN);
    }

    logging::logf(logging::log_level::TRACE, "network: %u wait for packet on socket %u\n", scheduler::get_pid(), socket_fd);

    if(socket.listen_packets.empty()){
        socket.listen_queue.sleep();
    }

    auto packet = socket.listen_packets.pop();
    std::copy_n(packet.payload, packet.payload_size, buffer);

    // The memory was allocated as a copy by the decoding process, it is safe to remove it here
    delete[] packet.payload;

    logging::logf(logging::log_level::TRACE, "network: %u received packet on socket %u\n", scheduler::get_pid(), socket_fd);

    return {packet.index};
}

std::expected<size_t> network::wait_for_packet(char* buffer, socket_fd_t socket_fd, size_t ms){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(!socket.listen){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_LISTEN);
    }

    logging::logf(logging::log_level::TRACE, "network: %u wait for packet on socket %u\n", scheduler::get_pid(), socket_fd);

    if(socket.listen_packets.empty()){
        if(!ms){
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_TIMEOUT);
        }

        if(!socket.listen_queue.sleep(ms)){
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_TIMEOUT);
        }
    }

    auto packet = socket.listen_packets.pop();
    std::copy_n(packet.payload, packet.payload_size, buffer);

    // The memory was allocated as a copy by the decoding process, it is safe to remove it here
    delete[] packet.payload;

    logging::logf(logging::log_level::TRACE, "network: %u received packet on socket %u\n", scheduler::get_pid(), socket_fd);

    return {packet.index};
}

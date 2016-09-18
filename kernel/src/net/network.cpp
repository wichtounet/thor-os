//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>
#include <atomic.hpp>
#include <bit_field.hpp>

#include "net/network.hpp"
#include "net/ethernet_layer.hpp"

#include "drivers/rtl8139.hpp"
#include "drivers/pci.hpp"
#include "drivers/loopback.hpp"

#include "physical_allocator.hpp"
#include "scheduler.hpp"
#include "logging.hpp"
#include "kernel_utils.hpp"

#include "fs/sysfs.hpp"

#include "tlib/errors.hpp"

#include "net/icmp_layer.hpp"
#include "net/dns_layer.hpp"
#include "net/udp_layer.hpp"
#include "net/tcp_layer.hpp"

namespace {

std::vector<network::interface_descriptor> interfaces;

void rx_thread(void* data){
    auto& interface = *reinterpret_cast<network::interface_descriptor*>(data);

    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "network: RX Thread for interface %u started (pid:%u)\n", interface.id, pid);

    while(true){
        interface.rx_sem.lock();

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
        interface.tx_sem.lock();

        auto packet = interface.tx_queue.pop();
        interface.hw_send(interface, packet);

        thor_assert(!packet.user);

        delete[] packet.payload;
    }
}

void sysfs_publish(const network::interface_descriptor& interface){
    auto p = path("/net") / interface.name;

    sysfs::set_constant_value(path("/sys"), p / "name", interface.name);
    sysfs::set_constant_value(path("/sys"), p / "driver", interface.driver);
    sysfs::set_constant_value(path("/sys"), p / "enabled", interface.enabled ? "true" : "false");
    sysfs::set_constant_value(path("/sys"), p / "pci_device", std::to_string(interface.pci_device));
    sysfs::set_constant_value(path("/sys"), p / "mac", std::to_string(interface.mac_address));

    if(interface.enabled){
        auto ip      = interface.ip_address;
        auto ip_addr = std::to_string(ip(0)) + "." + std::to_string(ip(1)) + "." + std::to_string(ip(2)) + "." + std::to_string(ip(3));

        sysfs::set_constant_value(path("/sys"), p / "ip", ip_addr);

        if (!interface.is_loopback()) {
            auto gateway      = interface.gateway;
            auto gateway_addr = std::to_string(gateway(0)) + "." + std::to_string(gateway(1)) + "." + std::to_string(gateway(2)) + "." + std::to_string(gateway(3));

            sysfs::set_constant_value(path("/sys"), p / "gateway", gateway_addr);
        }
    }
}

network::socket_protocol datagram_protocol(network::socket_protocol protocol){
    switch(protocol){
        case network::socket_protocol::DNS:
            return network::socket_protocol::UDP;

        default:
            return protocol;
    }
}

network::socket_protocol stream_protocol(network::socket_protocol protocol){
    switch(protocol){
        default:
            return protocol;
    }
}

} //end of anonymous namespace

void network::init(){
    size_t index = 0;

    for(size_t i = 0; i < pci::number_of_devices(); ++i){
        auto& pci_device = pci::device(i);

        if(pci_device.class_type == pci::device_class_type::NETWORK){
            auto& interface = interfaces.emplace_back();

            interface.id = interfaces.size() - 1;
            interface.name = std::string("net") + std::to_string(index);
            interface.pci_device = i;
            interface.enabled = false;
            interface.driver = "";
            interface.driver_data = nullptr;

            if(pci_device.vendor_id == 0x10EC && pci_device.device_id == 0x8139){
                interface.enabled = true;
                interface.driver = "rtl8139";

                rtl8139::init_driver(interface, pci_device);
            }

            if(interface.enabled){
                //TODO This should be configurable
                interface.ip_address = network::ip::make_address(10, 0, 2, 15);
                interface.gateway    = network::ip::make_address(10, 0, 2, 2);

                interface.tx_lock.init(1);
                interface.tx_sem.init(0);
                interface.rx_sem.init(0);
            }

            sysfs_publish(interface);

            ++index;
        }
    }

    // Install the loopback device

    auto& interface = interfaces.emplace_back();

    interface.id          = interfaces.size() - 1;
    interface.name        = "loopback";
    interface.pci_device  = 0;
    interface.enabled     = true;
    interface.driver      = "loopback";
    interface.driver_data = nullptr;
    interface.ip_address  = network::ip::make_address(127, 0, 0, 1);

    interface.tx_lock.init(1);
    interface.tx_sem.init(0);
    interface.rx_sem.init(0);

    loopback::init_driver(interface);

    sysfs_publish(interface);

    for(auto& interface : interfaces){
        if(interface.enabled){
            if(interface.is_loopback()){
                loopback::finalize_driver(interface);
            } else if(interface.driver == "rtl8139"){
                rtl8139::finalize_driver(interface);
            }
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

            auto rx_name = "net_rx_" + interface.name;
            auto tx_name = "net_tx_" + interface.name;

            auto& rx_process = scheduler::create_kernel_task_args(rx_name.c_str(), rx_user_stack, rx_kernel_stack, &rx_thread, &interface);
            auto& tx_process = scheduler::create_kernel_task_args(tx_name.c_str(), tx_user_stack, tx_kernel_stack, &tx_thread, &interface);

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

network::interface_descriptor& network::select_interface(network::ip::address address){
    if(address == network::ip::make_address(127, 0, 0, 1)){
        for(auto& interface : interfaces){
            if(interface.enabled && interface.is_loopback()){
                return interface;
            }
        }
    }

    // Otherwise return the first enabled interface

    for(auto& interface : interfaces){
        if(interface.enabled){
            return interface;
        }
    }

    thor_unreachable("network: Should never happen");
}

std::expected<network::socket_fd_t> network::open(network::socket_domain domain, network::socket_type type, network::socket_protocol protocol){
    // Make sure the socket domain is valid
    if(domain != socket_domain::AF_INET){
        return std::make_expected_from_error<network::socket_fd_t>(std::ERROR_SOCKET_INVALID_DOMAIN);
    }

    // Make sure the socket type is valid
    if(type != socket_type::RAW && type != socket_type::DGRAM && type != socket_type::STREAM){
        return std::make_expected_from_error<network::socket_fd_t>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    // Make sure the socket protocol is valid
    if(protocol != socket_protocol::ICMP && protocol != socket_protocol::DNS && protocol != socket_protocol::TCP){
        return std::make_expected_from_error<network::socket_fd_t>(std::ERROR_SOCKET_INVALID_PROTOCOL);
    }

    // Make sure the socket protocol is valid for the given socket type
    if(type == socket_type::DGRAM && !(protocol == socket_protocol::DNS)){
        return std::make_expected_from_error<network::socket_fd_t>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }

    // Make sure the socket protocol is valid for the given socket type
    if(type == socket_type::STREAM && !(protocol == socket_protocol::TCP)){
        return std::make_expected_from_error<network::socket_fd_t>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
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

    if(!network::number_of_interfaces()){
        return {-std::ERROR_SOCKET_NO_INTERFACE, 0};
    }

    auto& socket = scheduler::get_socket(socket_fd);

    auto return_from_packet = [&socket](std::expected<network::ethernet::packet>& packet) -> std::tuple<size_t, size_t> {
        if (packet) {
            auto fd = socket.register_packet(*packet);

            return {fd, packet->index};
        } else {
            return {-packet.error(), 0};
        }
    };

    switch (socket.protocol) {
        case network::socket_protocol::ICMP: {
            auto descriptor = static_cast<network::icmp::packet_descriptor*>(desc);
            auto packet     = network::icmp::user_prepare_packet(buffer, socket, descriptor);

            return return_from_packet(packet);
        }

        case network::socket_protocol::UDP: {
            auto descriptor = static_cast<network::udp::packet_descriptor*>(desc);
            auto packet     = network::udp::user_prepare_packet(buffer, socket, descriptor);

            return return_from_packet(packet);
        }

        case network::socket_protocol::TCP: {
            auto descriptor = static_cast<network::tcp::packet_descriptor*>(desc);
            auto packet     = network::tcp::user_prepare_packet(buffer, socket, descriptor);

            return return_from_packet(packet);
        }

        case network::socket_protocol::DNS: {
            auto descriptor = static_cast<network::dns::packet_descriptor*>(desc);
            auto packet     = network::dns::user_prepare_packet(buffer, socket, descriptor);

            return return_from_packet(packet);
        }

        default:
            return {-std::ERROR_SOCKET_UNIMPLEMENTED, 0};
    }
}

std::expected<void> network::finalize_packet(socket_fd_t socket_fd, size_t packet_fd){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(!socket.has_packet(packet_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_PACKET_FD);
    }

    auto& packet = socket.get_packet(packet_fd);
    auto& interface = network::interface(packet.interface);

    auto check_and_return = [&socket, &packet_fd](const std::expected<void>& ret){
        if(ret){
            socket.erase_packet(packet_fd);
        }

        return ret;
    };

    switch(socket.protocol){
        case network::socket_protocol::ICMP:
            return check_and_return(network::icmp::finalize_packet(interface, packet));

        case network::socket_protocol::TCP:
            return check_and_return(network::tcp::finalize_packet(interface, packet));

        case network::socket_protocol::UDP:
            return check_and_return(network::udp::finalize_packet(interface, packet));

        case network::socket_protocol::DNS:
            return check_and_return(network::dns::finalize_packet(interface, packet));

        default:
            return std::make_unexpected<void>(std::ERROR_SOCKET_UNIMPLEMENTED);
    }

}

std::expected<void> network::listen(socket_fd_t socket_fd, bool listen){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    socket.listen = listen;

    return std::make_expected();
}

std::expected<size_t> network::client_bind(socket_fd_t socket_fd, network::ip::address address){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::DGRAM){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    switch(datagram_protocol(socket.protocol)){
        case socket_protocol::UDP:
            return network::udp::client_bind(socket, 53, address);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
}

std::expected<void> network::client_unbind(socket_fd_t socket_fd){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::DGRAM){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    logging::logf(logging::log_level::TRACE, "network: %u disconnect from datagram socket %u\n", scheduler::get_pid(), socket_fd);

    switch(datagram_protocol(socket.protocol)){
        case network::socket_protocol::UDP:
            return network::udp::client_unbind(socket);

        default:
            return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
}

std::expected<size_t> network::connect(socket_fd_t socket_fd, network::ip::address server, size_t port){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::STREAM){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    switch(stream_protocol(socket.protocol)){
        case socket_protocol::TCP:
            return network::tcp::connect(socket, select_interface(server), port, server);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
}

std::expected<void> network::disconnect(socket_fd_t socket_fd){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::STREAM){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    logging::logf(logging::log_level::TRACE, "network: %u disconnect from stream socket %u\n", scheduler::get_pid(), socket_fd);

    switch(datagram_protocol(socket.protocol)){
        case network::socket_protocol::UDP:
            return network::tcp::disconnect(socket);

        default:
            return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
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
        socket.listen_queue.wait();
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

        if(!socket.listen_queue.wait_for(ms)){
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

void network::propagate_packet(const ethernet::packet& packet, socket_protocol protocol){
    // TODO Need something better for this

    for(size_t pid = 0; pid < scheduler::MAX_PROCESS; ++pid){
        auto state = scheduler::get_process_state(pid);
        if(state != scheduler::process_state::EMPTY && state != scheduler::process_state::NEW && state != scheduler::process_state::KILLED){
            for(auto& socket : scheduler::get_sockets(pid)){
                if(socket.listen){
                    bool propagate = false;
                    if(socket.type == socket_type::RAW){
                        if(socket.protocol == protocol){
                            propagate = true;
                        }
                    }

                    // Note: Stream and datagram sockets are responsible for propagation

                    if (propagate) {
                        auto copy    = packet;
                        copy.payload = new char[copy.payload_size];
                        std::copy_n(packet.payload, packet.payload_size, copy.payload);

                        socket.listen_packets.push(copy);
                        socket.listen_queue.notify_one();
                    }
                }
            }
        }
    }
}

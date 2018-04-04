//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
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
#include "net/arp_layer.hpp"
#include "net/dhcp_layer.hpp"
#include "net/icmp_layer.hpp"
#include "net/dns_layer.hpp"
#include "net/udp_layer.hpp"
#include "net/tcp_layer.hpp"

#include "drivers/rtl8139.hpp"
#include "drivers/pci.hpp"
#include "drivers/loopback.hpp"

#include "physical_allocator.hpp"
#include "scheduler.hpp"
#include "logging.hpp"
#include "kernel_utils.hpp"

#include "fs/sysfs.hpp"

#include "tlib/errors.hpp"

/*
 * TODO: Network layers
 *
 * Ideally, we should not have a switch for each network layer function that has
 * the same think in each body. Solutions are:
 *  - Generation by macro :(
 *  - Generation by template
 *  - Virtualization :(
 *
 *  Problem with template solution is that not all layers have the same
 *  interface
 */

namespace {

std::vector<network::interface_descriptor> interfaces;

network::ip::address dns_address;

network::ethernet::layer* ethernet_layer;
network::arp::layer* arp_layer;
network::ip::layer* ip_layer;
network::icmp::layer* icmp_layer;
network::udp::layer* udp_layer;
network::dns::layer* dns_layer;
network::dhcp::layer* dhcp_layer;
network::tcp::layer* tcp_layer;

void rx_thread(void* data){
    auto& interface = *reinterpret_cast<network::interface_descriptor*>(data);

    auto pid = scheduler::get_pid();

    interface.rx_sem.claim();

    logging::logf(logging::log_level::TRACE, "network: RX Thread for interface %u started (pid:%u)\n", interface.id, pid);

    while(true){
        interface.rx_sem.wait();

        //TODO This is highly unsafe since back() and pop_back() can suffer interleaving

        auto packet = interface.rx_queue.top();
        interface.rx_queue.pop();

        ethernet_layer->decode(interface, packet);

        ++interface.rx_packets_counter;
        interface.rx_bytes_counter += packet->payload_size;
    }
}

void tx_thread(void* data){
    auto& interface = *reinterpret_cast<network::interface_descriptor*>(data);

    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "network: TX Thread for interface %u started (pid:%u)\n", interface.id, pid);

    while(true){
        interface.tx_sem.lock();

        auto packet = interface.tx_queue.top();
        interface.tx_queue.pop();
        interface.hw_send(interface, packet);

        thor_assert(!packet->user);

        ++interface.tx_packets_counter;
        interface.tx_bytes_counter += packet->payload_size;
    }
}

std::string sysfs_rx_packets(void* data){
    auto& interface = *reinterpret_cast<network::interface_descriptor*>(data);
    return std::to_string(interface.rx_packets_counter);
}

std::string sysfs_rx_bytes(void* data){
    auto& interface = *reinterpret_cast<network::interface_descriptor*>(data);
    return std::to_string(interface.rx_bytes_counter);
}

std::string sysfs_tx_packets(void* data){
    auto& interface = *reinterpret_cast<network::interface_descriptor*>(data);
    return std::to_string(interface.tx_packets_counter);
}

std::string sysfs_tx_bytes(void* data){
    auto& interface = *reinterpret_cast<network::interface_descriptor*>(data);
    return std::to_string(interface.tx_bytes_counter);
}

void sysfs_publish(network::interface_descriptor& interface){
    auto p = path("/net") / interface.name;

    sysfs::set_constant_value(sysfs::get_sys_path(), p / "name", interface.name);
    sysfs::set_constant_value(sysfs::get_sys_path(), p / "driver", interface.driver);
    sysfs::set_constant_value(sysfs::get_sys_path(), p / "enabled", interface.enabled ? "true" : "false");
    sysfs::set_constant_value(sysfs::get_sys_path(), p / "pci_device", std::to_string(interface.pci_device));
    sysfs::set_constant_value(sysfs::get_sys_path(), p / "mac", std::to_string(interface.mac_address));

    if(interface.enabled){
        auto ip      = interface.ip_address;
        auto ip_addr = std::to_string(ip(0)) + "." + std::to_string(ip(1)) + "." + std::to_string(ip(2)) + "." + std::to_string(ip(3));

        sysfs::set_constant_value(sysfs::get_sys_path(), p / "ip", ip_addr);

        if (!interface.is_loopback()) {
            auto gateway      = interface.gateway;
            auto gateway_addr = std::to_string(gateway(0)) + "." + std::to_string(gateway(1)) + "." + std::to_string(gateway(2)) + "." + std::to_string(gateway(3));

            sysfs::set_constant_value(sysfs::get_sys_path(), p / "gateway", gateway_addr);
        }

        sysfs::set_dynamic_value_data(sysfs::get_sys_path(), p / "rx_packets", sysfs_rx_packets, &interface);
        sysfs::set_dynamic_value_data(sysfs::get_sys_path(), p / "rx_bytes", sysfs_rx_bytes, &interface);
        sysfs::set_dynamic_value_data(sysfs::get_sys_path(), p / "tx_packets", sysfs_tx_packets, &interface);
        sysfs::set_dynamic_value_data(sysfs::get_sys_path(), p / "tx_bytes", sysfs_tx_bytes, &interface);
    }
}

size_t datagram_port(network::socket_protocol protocol){
    switch(protocol){
        case network::socket_protocol::DNS:
            return 53;

        default:
            return 0;
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

void network_discovery() {
    for (auto& interface : interfaces) {
        if (interface.enabled) {
            if (!interface.is_loopback()) {
                auto ip = dhcp_layer->request_ip(interface);

                if (ip) {
                    interface.ip_address = ip->ip_address;

                    logging::logf(logging::log_level::TRACE, "network: interface %u acquired IP %h by DHCP\n", interface.id, size_t(interface.ip_address.raw_address));

                    if (ip->gateway) {
                        interface.gateway = ip->ip_address;
                        logging::logf(logging::log_level::TRACE, "network: interface %u acquired gateway %h by DHCP\n", interface.id, size_t(interface.gateway.raw_address));
                    } else {
                        interface.gateway    = network::ip::make_address(10, 0, 2, 2);
                    }

                    if (ip->dns) {
                        dns_address = ip->dns_address;
                        logging::logf(logging::log_level::TRACE, "network: acquired DNS %h by DHCP\n", size_t(interface.gateway.raw_address));
                    } else {
                        dns_address = network::ip::make_address(10, 0, 2, 2);
                    }
                } else {
                    logging::logf(logging::log_level::TRACE, "network: impossible to acquire address for interface %u\n", interface.id);

                    // Defaults for Qemu (better than nothing)
                    interface.ip_address = network::ip::make_address(10, 0, 2, 15);
                    interface.gateway    = network::ip::make_address(10, 0, 2, 2);

                    dns_address = network::ip::make_address(10, 0, 2, 2);
                }
            }
        }

        sysfs_publish(interface);
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

            if(pci_device.vendor_id == 0x10EC && pci_device.device_id == 0x8139){
                interface.enabled = true;
                interface.driver = "rtl8139";

                rtl8139::init_driver(interface, pci_device);
            }

            // No IP address by default
            interface.ip_address = network::ip::make_address(0, 0, 0, 0);

            if(interface.enabled){
                interface.tx_lock.init(1);
                interface.tx_sem.init(0);
            }

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
    interface.ip_address  = network::ip::make_address(127, 0, 0, 1);

    interface.tx_lock.init(1);
    interface.tx_sem.init(0);

    loopback::init_driver(interface);

    for(auto& interface : interfaces){
        if(interface.enabled){
            if(interface.is_loopback()){
                loopback::finalize_driver(interface);
            } else if(interface.driver == "rtl8139"){
                rtl8139::finalize_driver(interface);
            }
        }
    }

    // Initialize the necessary network layers

    ethernet_layer = new network::ethernet::layer();

    arp_layer = new network::arp::layer(ethernet_layer);
    ip_layer = new network::ip::layer(ethernet_layer);
    ip_layer->register_arp_layer(arp_layer);

    icmp_layer = new network::icmp::layer(ip_layer);

    udp_layer = new network::udp::layer(ip_layer);
    dns_layer = new network::dns::layer(udp_layer);
    dhcp_layer = new network::dhcp::layer(udp_layer);

    tcp_layer = new network::tcp::layer(ip_layer);
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

    scheduler::queue_async_init_task(network_discovery);
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
    if(protocol != socket_protocol::ICMP && protocol != socket_protocol::DNS && protocol != socket_protocol::TCP && protocol != socket_protocol::UDP){
        return std::make_expected_from_error<network::socket_fd_t>(std::ERROR_SOCKET_INVALID_PROTOCOL);
    }

    // Make sure the socket protocol is valid for the given socket type
    if(type == socket_type::DGRAM && !(protocol == socket_protocol::DNS || protocol == socket_protocol::UDP)){
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

    auto return_from_packet = [&socket](std::expected<network::packet_p>& packet) -> std::tuple<size_t, size_t> {
        if (packet) {
            auto fd = socket.register_packet(*packet);

            return {fd, (*packet)->index};
        } else {
            return {-packet.error(), 0};
        }
    };

    switch (socket.protocol) {
        case network::socket_protocol::ICMP: {
            auto descriptor = static_cast<network::icmp::packet_descriptor*>(desc);
            auto packet     = icmp_layer->user_prepare_packet(buffer, socket, descriptor);

            return return_from_packet(packet);
        }

        case network::socket_protocol::UDP: {
            auto descriptor = static_cast<network::udp::packet_descriptor*>(desc);
            auto packet     = udp_layer->user_prepare_packet(buffer, socket, descriptor);

            return return_from_packet(packet);
        }

        case network::socket_protocol::TCP: {
            auto descriptor = static_cast<network::tcp::packet_descriptor*>(desc);
            auto packet     = tcp_layer->user_prepare_packet(buffer, socket, descriptor);

            return return_from_packet(packet);
        }

        case network::socket_protocol::DNS: {
            auto descriptor = static_cast<network::dns::packet_descriptor*>(desc);
            auto packet     = dns_layer->user_prepare_packet(buffer, socket, descriptor);

            return return_from_packet(packet);
        }

        default:
            return {-std::ERROR_SOCKET_UNIMPLEMENTED, 0};
    }
}

std::expected<void> network::send(socket_fd_t socket_fd, const char* buffer, size_t n, char* target_buffer){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    if(!network::number_of_interfaces()){
        return std::make_unexpected<void>(std::ERROR_SOCKET_NO_INTERFACE);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    switch (socket.protocol) {
        case network::socket_protocol::TCP:
            return tcp_layer->send(target_buffer, socket, buffer, n);

        case network::socket_protocol::UDP:
            return udp_layer->send(target_buffer, socket, buffer, n);

        default:
            return std::make_unexpected<void>(std::ERROR_SOCKET_UNIMPLEMENTED);
    }
}

std::expected<void> network::send_to(socket_fd_t socket_fd, const char* buffer, size_t n, char* target_buffer, void* address){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    if(!network::number_of_interfaces()){
        return std::make_unexpected<void>(std::ERROR_SOCKET_NO_INTERFACE);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    switch (socket.protocol) {
        case network::socket_protocol::UDP:
            return udp_layer->send_to(target_buffer, socket, buffer, n, address);

        default:
            return std::make_unexpected<void>(std::ERROR_SOCKET_UNIMPLEMENTED);
    }
}

std::expected<size_t> network::receive(socket_fd_t socket_fd, char* buffer, size_t n){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    if(!network::number_of_interfaces()){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NO_INTERFACE);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(!socket.listen){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_LISTEN);
    }

    switch (socket.protocol) {
        case network::socket_protocol::UDP:
            return udp_layer->receive(buffer, socket, n);

        case network::socket_protocol::TCP:
            return tcp_layer->receive(buffer, socket, n);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_UNIMPLEMENTED);
    }
}

std::expected<size_t> network::receive(socket_fd_t socket_fd, char* buffer, size_t n, size_t ms){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    if(!network::number_of_interfaces()){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NO_INTERFACE);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(!socket.listen){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_LISTEN);
    }

    switch (socket.protocol) {
        case network::socket_protocol::UDP:
            return udp_layer->receive(buffer, socket, n, ms);

        case network::socket_protocol::TCP:
            return tcp_layer->receive(buffer, socket, n, ms);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_UNIMPLEMENTED);
    }
}

std::expected<size_t> network::receive_from(socket_fd_t socket_fd, char* buffer, size_t n, void* address){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    if(!network::number_of_interfaces()){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NO_INTERFACE);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(!socket.listen){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_LISTEN);
    }

    switch (socket.protocol) {
        case network::socket_protocol::UDP:
            return udp_layer->receive_from(buffer, socket, n, address);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_UNIMPLEMENTED);
    }
}

std::expected<size_t> network::receive_from(socket_fd_t socket_fd, char* buffer, size_t n, size_t ms, void* address){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    if(!network::number_of_interfaces()){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NO_INTERFACE);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(!socket.listen){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_LISTEN);
    }

    switch (socket.protocol) {
        case network::socket_protocol::UDP:
            return udp_layer->receive_from(buffer, socket, n, ms, address);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_UNIMPLEMENTED);
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
    auto& interface = network::interface(packet->interface);

    auto check_and_return = [&socket, &packet_fd](const std::expected<void>& ret){
        if(ret){
            socket.erase_packet(packet_fd);
        }

        return ret;
    };

    switch(socket.protocol){
        case network::socket_protocol::ICMP:
            return check_and_return(icmp_layer->finalize_packet(interface, socket, packet));

        case network::socket_protocol::TCP:
            return check_and_return(tcp_layer->finalize_packet(interface, socket, packet));

        case network::socket_protocol::UDP:
            return check_and_return(udp_layer->finalize_packet(interface, socket, packet));

        case network::socket_protocol::DNS:
            return check_and_return(dns_layer->finalize_packet(interface, socket, packet));

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

    auto port = datagram_port(socket.protocol);

    if(!port){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }

    switch(datagram_protocol(socket.protocol)){
        case socket_protocol::UDP:
            return udp_layer->client_bind(socket, port, address);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
}

std::expected<size_t> network::client_bind(socket_fd_t socket_fd, network::ip::address address, size_t port){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::DGRAM){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    switch(datagram_protocol(socket.protocol)){
        case socket_protocol::UDP:
            return udp_layer->client_bind(socket, port, address);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
}

std::expected<void> network::server_bind(socket_fd_t socket_fd, network::ip::address address){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::DGRAM){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    auto port = datagram_port(socket.protocol);

    if(!port){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }

    switch(datagram_protocol(socket.protocol)){
        case socket_protocol::UDP:
            return udp_layer->server_bind(socket, port, address);

        default:
            return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
}

std::expected<void> network::server_bind(socket_fd_t socket_fd, network::ip::address address, size_t port){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::DGRAM){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    switch(datagram_protocol(socket.protocol)){
        case socket_protocol::UDP:
            return udp_layer->server_bind(socket, port, address);

        default:
            return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
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
            return udp_layer->client_unbind(socket);

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
            return tcp_layer->connect(socket, select_interface(server), port, server);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
}

std::expected<void> network::server_start(socket_fd_t socket_fd, network::ip::address server, size_t port){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::STREAM){
        return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    switch(stream_protocol(socket.protocol)){
        case socket_protocol::TCP:
            return tcp_layer->server_start(socket, port, server);

        default:
            return std::make_unexpected<void>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
}

std::expected<size_t> network::accept(socket_fd_t socket_fd){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::STREAM){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    switch(stream_protocol(socket.protocol)){
        case socket_protocol::TCP:
            return tcp_layer->accept(socket);

        default:
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE_PROTOCOL);
    }
}

std::expected<size_t> network::accept(socket_fd_t socket_fd, size_t ms){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(socket.type != socket_type::STREAM){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_TYPE);
    }

    switch(stream_protocol(socket.protocol)){
        case socket_protocol::TCP:
            return tcp_layer->accept(socket, ms);

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
        case network::socket_protocol::TCP:
            return tcp_layer->disconnect(socket);

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

    auto packet = socket.listen_packets.top();
    socket.listen_packets.pop();

    std::copy_n(packet->payload, packet->payload_size, buffer);

    logging::logf(logging::log_level::TRACE, "network: %u received packet on socket %u\n", scheduler::get_pid(), socket_fd);

    return {packet->index};
}

std::expected<size_t> network::wait_for_packet(char* buffer, socket_fd_t socket_fd, size_t ms){
    if(!scheduler::has_socket(socket_fd)){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_INVALID_FD);
    }

    auto& socket = scheduler::get_socket(socket_fd);

    if(!socket.listen){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_LISTEN);
    }

    logging::logf(logging::log_level::TRACE, "network: %u wait for packet on socket (with timeout) %u\n", scheduler::get_pid(), socket_fd);

    if(socket.listen_packets.empty()){
        if(!ms){
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_TIMEOUT);
        }

        if(!socket.listen_queue.wait_for(ms)){
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_TIMEOUT);
        }
    }

    auto packet = socket.listen_packets.top();
    socket.listen_packets.pop();

    std::copy_n(packet->payload, packet->payload_size, buffer);

    logging::logf(logging::log_level::TRACE, "network: %u received packet on socket %u\n", scheduler::get_pid(), socket_fd);

    return {packet->index};
}

void network::propagate_packet(const packet_p& packet, socket_protocol protocol){
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
                        socket.listen_packets.push(packet);
                        socket.listen_queue.notify_one();
                    }
                }
            }
        }
    }
}

network::ip::address network::dns_server(){
    return dns_address;
}

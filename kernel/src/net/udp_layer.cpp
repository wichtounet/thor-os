//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/errors.hpp"

#include "net/udp_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/dns_layer.hpp"
#include "net/dhcp_layer.hpp"
#include "net/checksum.hpp"
#include "net/network.hpp"

#include "kernel_utils.hpp"

namespace {

void compute_checksum(network::packet& packet){
    auto* ip_header = reinterpret_cast<network::ip::header*>(packet.payload + packet.tag(1));
    auto* udp_header = reinterpret_cast<network::udp::header*>(packet.payload + packet.tag(2));

    udp_header->checksum = 0;

    auto length = switch_endian_16(udp_header->length);

    uint32_t sum = 0;

    // Accumulate the Payload
    sum += network::checksum_add_bytes(packet.payload + packet.tag(2), length);

    // Accumulate the IP addresses
    sum += network::checksum_add_bytes(&ip_header->source_ip, 8);

    // Accumulate the IP Protocol
    sum += ip_header->protocol;

    // Accumulate the UDP length
    sum += length;

    // Complete the 1-complement sum
    udp_header->checksum = switch_endian_16(network::checksum_finalize_nz(sum));
}

void prepare_packet(network::packet& packet, size_t source, size_t target, size_t payload_size){
    packet.tag(2, packet.index);

    // Set the UDP header

    auto* udp_header = reinterpret_cast<network::udp::header*>(packet.payload + packet.index);

    udp_header->source_port = switch_endian_16(source);
    udp_header->target_port = switch_endian_16(target);
    udp_header->length      = switch_endian_16(sizeof(network::udp::header) + payload_size);

    packet.index += sizeof(network::udp::header);
}

} //end of anonymous namespace

network::udp::layer::layer(network::ip::layer* parent) : parent(parent) {
    parent->register_udp_layer(this);

    // The first port will be 1024
    local_port = 1023;
}

void network::udp::layer::decode(network::interface_descriptor& interface, network::packet_p& packet){
    packet->tag(2, packet->index);

    auto* udp_header = reinterpret_cast<header*>(packet->payload + packet->index);

    logging::logf(logging::log_level::TRACE, "udp: Start UDP packet handling\n");

    auto source_port = switch_endian_16(udp_header->source_port);
    auto target_port = switch_endian_16(udp_header->target_port);
    auto length      = switch_endian_16(udp_header->length);

    logging::logf(logging::log_level::TRACE, "udp: Source Port %h \n", source_port);
    logging::logf(logging::log_level::TRACE, "udp: Target Port %h \n", target_port);
    logging::logf(logging::log_level::TRACE, "udp: Length %h \n", length);

    packet->index += sizeof(header);

    if (source_port == 53) {
        dns_layer->decode(interface, packet);
    } else if (source_port == 67) {
        dhcp_layer->decode(interface, packet);
    }

    auto connection_ptr = connections.get_connection_for_packet(source_port, target_port);

    if(connection_ptr){
        auto& connection = *connection_ptr;

        // Propagate to the kernel socket

        if (connection.socket) {
            auto& socket = *connection.socket;

            if (socket.listen) {
                socket.listen_packets.push(packet);
                socket.listen_queue.notify_one();
            }
        }
    } else {
        logging::logf(logging::log_level::DEBUG, "udp: Received packet for which there are no connection\n");
    }
}

std::expected<network::packet_p> network::udp::layer::kernel_prepare_packet(network::interface_descriptor& interface, const kernel_packet_descriptor& descriptor){
    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{sizeof(header) + descriptor.payload_size, descriptor.target_ip, 0x11};
    auto packet = parent->kernel_prepare_packet(interface, desc);

    if(packet){
        ::prepare_packet(**packet, descriptor.source_port, descriptor.target_port, descriptor.payload_size);
    }

    return packet;
}

std::expected<network::packet_p> network::udp::layer::user_prepare_packet(char* buffer, network::socket& sock, const packet_descriptor* descriptor){
    auto& connection = sock.get_connection_data<udp_connection>();

    auto ip = connection.server_address;
    auto ip_str = network::ip::ip_to_str(ip);
    logging::logf(logging::log_level::ERROR, "udp: Craft destination=%s\n", ip_str.c_str());

    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{sizeof(header) + descriptor->payload_size, connection.server_address, 0x11};
    auto packet = parent->user_prepare_packet(buffer, network::select_interface(connection.server_address), &desc);

    if(packet){
        ::prepare_packet(**packet, connection.local_port, connection.server_port, descriptor->payload_size);
    }

    return packet;
}

std::expected<network::packet_p> network::udp::layer::user_prepare_packet(char* buffer, network::socket& sock, const network::udp::packet_descriptor* descriptor, network::inet_address* address){
    auto& connection = sock.get_connection_data<udp_connection>();

    auto ip = connection.server_address;
    auto ip_str = network::ip::ip_to_str(ip);
    logging::logf(logging::log_level::ERROR, "udp: Craft destination=%s\n", ip_str.c_str());

    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{sizeof(network::udp::header) + descriptor->payload_size, address->address, 0x11};
    auto packet = parent->user_prepare_packet(buffer, network::select_interface(connection.server_address), &desc);

    if(packet){
        ::prepare_packet(**packet, connection.server_port, address->port, descriptor->payload_size);
    }

    return packet;
}

std::expected<void> network::udp::layer::finalize_packet(network::interface_descriptor& interface, network::packet_p& p){
    p->index -= sizeof(header);

    // Compute the checksum
    compute_checksum(*p);

    // Give the packet to the IP layer for finalization
    return parent->finalize_packet(interface, p);
}

std::expected<void> network::udp::layer::finalize_packet(network::interface_descriptor& interface, network::socket& /*sock*/, network::packet_p& p){
    return this->finalize_packet(interface, p);
}

std::expected<size_t> network::udp::layer::client_bind(network::socket& sock, size_t server_port, network::ip::address server){
    // Create the connection

    auto& connection = connections.create_connection();

    connection.local_port     = ++local_port;
    connection.server_port    = server_port;
    connection.server_address = server;

    // Link the socket and connection
    sock.connection_data = &connection;
    connection.socket = &sock;

    // Mark the connection as connected

    connection.connected = true;

    return {connection.local_port};
}

std::expected<void> network::udp::layer::server_bind(network::socket& sock, size_t server_port, network::ip::address server){
    // Create the connection

    auto& connection = connections.create_connection();

    connection.server_port    = server_port;
    connection.server_address = server;
    connection.server         = true;

    // Link the socket and connection
    sock.connection_data = &connection;
    connection.socket = &sock;

    // Mark the connection as connected

    connection.connected = true;

    return {};
}

std::expected<void> network::udp::layer::client_unbind(network::socket& sock){
    auto& connection = sock.get_connection_data<udp_connection>();

    if(!connection.connected){
        return std::make_unexpected<void>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    // Mark the connection as not connected

    connection.connected = false;

    connections.remove_connection(connection);

    return {};
}

std::expected<void> network::udp::layer::send(char* target_buffer, network::socket& socket, const char* buffer, size_t n){
    auto& connection = socket.get_connection_data<udp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<void>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    network::udp::packet_descriptor desc{n};
    auto packet_e = user_prepare_packet(target_buffer, socket, &desc);

    if (packet_e) {
        auto& packet = *packet_e;

        for(size_t i = 0; i < n; ++i){
            packet->payload[packet->index + i] = buffer[i];
        }

        auto target_ip  = connection.server_address;
        auto& interface = network::select_interface(target_ip);
        return finalize_packet(interface, packet);
    }

    return std::make_unexpected<void>(packet_e.error());
}

std::expected<void> network::udp::layer::send_to(char* target_buffer, network::socket& socket, const char* buffer, size_t n, void* addr){
    auto& connection = socket.get_connection_data<udp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<void>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    auto* address = static_cast<network::inet_address*>(addr);

    network::udp::packet_descriptor desc{n};
    auto packet_e = user_prepare_packet(target_buffer, socket, &desc, address);

    if (packet_e) {
        auto& packet = *packet_e;

        for(size_t i = 0; i < n; ++i){
            packet->payload[packet->index + i] = buffer[i];
        }

        auto target_ip  = connection.server_address;
        auto& interface = network::select_interface(target_ip);
        return finalize_packet(interface, packet);
    }

    return std::make_unexpected<void>(packet_e.error());
}

std::expected<size_t> network::udp::layer::receive(char* buffer, network::socket& socket, size_t n){
    auto& connection = socket.get_connection_data<udp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    if(socket.listen_packets.empty()){
        socket.listen_queue.wait();
    }

    auto packet = socket.listen_packets.top();
    socket.listen_packets.pop();

    auto* udp_header = reinterpret_cast<network::udp::header*>(packet->payload + packet->tag(2));
    auto payload_len = switch_endian_16(udp_header->length);

    if(payload_len > n){
        return std::make_unexpected<size_t>(std::ERROR_BUFFER_SMALL);
    }

    std::copy_n(packet->payload + packet->index, payload_len, buffer);

    return payload_len;
}

std::expected<size_t> network::udp::layer::receive(char* buffer, network::socket& socket, size_t n, size_t ms){
    auto& connection = socket.get_connection_data<udp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

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

    auto* udp_header = reinterpret_cast<network::udp::header*>(packet->payload + packet->tag(2));
    auto payload_len = switch_endian_16(udp_header->length);

    if(payload_len > n){
        return std::make_unexpected<size_t>(std::ERROR_BUFFER_SMALL);
    }

    std::copy_n(packet->payload + packet->index, payload_len, buffer);

    return payload_len;
}

std::expected<size_t> network::udp::layer::receive_from(char* buffer, network::socket& socket, size_t n, void* addr){
    auto& connection = socket.get_connection_data<udp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    auto* address = static_cast<network::inet_address*>(addr);

    if(socket.listen_packets.empty()){
        socket.listen_queue.wait();
    }

    auto packet = socket.listen_packets.top();
    socket.listen_packets.pop();

    auto* udp_header = reinterpret_cast<network::udp::header*>(packet->payload + packet->tag(2));
    auto payload_len = switch_endian_16(udp_header->length);

    if(payload_len > n){
        return std::make_unexpected<size_t>(std::ERROR_BUFFER_SMALL);
    }

    auto* ip_header = reinterpret_cast<network::ip::header*>(packet->payload + packet->tag(1));

    address->port    = switch_endian_16(udp_header->source_port);
    address->address = switch_endian_32(ip_header->source_ip);

    std::copy_n(packet->payload + packet->index, payload_len, buffer);

    return payload_len;
}

std::expected<size_t> network::udp::layer::receive_from(char* buffer, network::socket& socket, size_t n, size_t ms, void* addr){
    auto& connection = socket.get_connection_data<udp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    auto* address = static_cast<network::inet_address*>(addr);

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

    auto* udp_header = reinterpret_cast<network::udp::header*>(packet->payload + packet->tag(2));
    auto payload_len = switch_endian_16(udp_header->length);

    if(payload_len > n){
        return std::make_unexpected<size_t>(std::ERROR_BUFFER_SMALL);
    }

    auto* ip_header = reinterpret_cast<network::ip::header*>(packet->payload + packet->tag(1));

    address->port    = switch_endian_16(udp_header->source_port);
    address->address = switch_endian_32(ip_header->source_ip);

    std::copy_n(packet->payload + packet->index, payload_len, buffer);

    return payload_len;
}

void network::udp::layer::register_dns_layer(network::dns::layer* layer){
    this->dns_layer = layer;
}

void network::udp::layer::register_dhcp_layer(network::dhcp::layer* layer){
    this->dhcp_layer = layer;
}

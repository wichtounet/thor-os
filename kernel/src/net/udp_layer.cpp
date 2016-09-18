//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/connection_handler.hpp"
#include "net/udp_layer.hpp"
#include "net/dns_layer.hpp"
#include "net/checksum.hpp"

#include "tlib/errors.hpp"

#include "kernel_utils.hpp"

namespace {

struct udp_connection {
    size_t local_port;                   ///< The local source port
    size_t server_port;                  ///< The server port
    network::ip::address server_address; ///< The server address

    bool connected = false;

    network::socket* socket = nullptr;
};

network::connection_handler<udp_connection> connections;

void compute_checksum(network::ethernet::packet& packet){
    auto* ip_header = reinterpret_cast<network::ip::header*>(packet.payload + packet.tag(1));
    auto* udp_header = reinterpret_cast<network::udp::header*>(packet.payload + packet.index);

    udp_header->checksum = 0;

    auto length = switch_endian_16(udp_header->length);

    // Accumulate the Payload
    auto sum = network::checksum_add_bytes(packet.payload + packet.index, length);

    // Accumulate the IP addresses
    sum += network::checksum_add_bytes(&ip_header->source_ip, 8);

    // Accumulate the IP Protocol
    sum += ip_header->protocol;

    // Accumulate the UDP length
    sum += length;

    // Complete the 1-complement sum
    udp_header->checksum = switch_endian_16(network::checksum_finalize_nz(sum));
}

void prepare_packet(network::ethernet::packet& packet, size_t source, size_t target, size_t payload_size){
    packet.tag(2, packet.index);

    // Set the UDP header

    auto* udp_header = reinterpret_cast<network::udp::header*>(packet.payload + packet.index);

    udp_header->source_port = switch_endian_16(source);
    udp_header->target_port = switch_endian_16(target);
    udp_header->length      = switch_endian_16(sizeof(network::udp::header) + payload_size);

    packet.index += sizeof(network::udp::header);
}

} //end of anonymous namespace

void network::udp::decode(network::interface_descriptor& interface, network::ethernet::packet& packet){
    packet.tag(2, packet.index);

    auto* udp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "udp: Start UDP packet handling\n");

    auto source_port = switch_endian_16(udp_header->source_port);
    auto target_port = switch_endian_16(udp_header->target_port);
    auto length      = switch_endian_16(udp_header->length);

    logging::logf(logging::log_level::TRACE, "udp: Source Port %h \n", source_port);
    logging::logf(logging::log_level::TRACE, "udp: Target Port %h \n", target_port);
    logging::logf(logging::log_level::TRACE, "udp: Length %h \n", length);

    packet.index += sizeof(header);

    if(source_port == 53){
        network::dns::decode(interface, packet);
    }
}

std::expected<network::ethernet::packet> network::udp::kernel_prepare_packet(network::interface_descriptor& interface, const packet_descriptor& descriptor){
    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{sizeof(header) + descriptor.payload_size, descriptor.target_ip, 0x11};
    auto packet = network::ip::kernel_prepare_packet(interface, desc);

    if(packet){
        ::prepare_packet(*packet, descriptor.source, descriptor.target, descriptor.payload_size);
    }

    return packet;
}

std::expected<network::ethernet::packet> network::udp::user_prepare_packet(char* buffer, network::interface_descriptor& interface, const packet_descriptor* descriptor){
    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{sizeof(header) + descriptor->payload_size, descriptor->target_ip, 0x11};
    auto packet = network::ip::user_prepare_packet(buffer, interface, &desc);

    if(packet){
        ::prepare_packet(*packet, descriptor->source, descriptor->target, descriptor->payload_size);
    }

    return packet;
}

std::expected<void> network::udp::finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p){
    p.index -= sizeof(header);

    // Compute the checksum
    compute_checksum(p);

    // Give the packet to the IP layer for finalization
    return network::ip::finalize_packet(interface, p);
}

std::expected<void> network::udp::client_bind(network::socket& sock, network::interface_descriptor& interface, size_t local_port, size_t server_port, network::ip::address server){
    // Create the connection

    auto& connection = connections.create_connection();

    connection.local_port     = local_port;
    connection.server_port    = server_port;
    connection.server_address = server;

    // Link the socket and connection
    sock.connection_data = &connection;
    connection.socket = &sock;

    // Mark the connection as connected

    connection.connected = true;

    return {};
}

std::expected<void> network::udp::client_unbind(network::socket& sock){
    auto& connection = sock.get_connection_data<udp_connection>();

    if(!connection.connected){
        return std::make_unexpected<void>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    // Mark the connection as not connected

    connection.connected = false;

    connections.remove_connection(connection);

    return {};
}

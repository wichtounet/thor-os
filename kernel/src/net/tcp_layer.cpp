//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/tcp_layer.hpp"
#include "net/dns_layer.hpp"

#include "kernel_utils.hpp"

namespace {

void compute_checksum(network::ethernet::packet& packet){
    auto* ip_header  = reinterpret_cast<network::ip::header*>(packet.payload + packet.tag(1));
    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet.payload + packet.index);

    tcp_header->checksum = 0;

    //TODO
}

void prepare_packet(network::ethernet::packet& packet, size_t source, size_t target, size_t payload_size){
    packet.tag(2, packet.index);

    // Set the TCP header

    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet.payload + packet.index);

    tcp_header->source_port = switch_endian_16(source);
    tcp_header->target_port = switch_endian_16(target);

    //TODO

    packet.index += sizeof(network::tcp::header);
}

} //end of anonymous namespace

void network::tcp::decode(network::interface_descriptor& interface, network::ethernet::packet& packet){
    packet.tag(2, packet.index);

    auto* tcp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "tcp: Start TCP packet handling\n");

    auto source_port = switch_endian_16(tcp_header->source_port);
    auto target_port = switch_endian_16(tcp_header->target_port);

    logging::logf(logging::log_level::TRACE, "tcp: Source Port %h \n", source_port);
    logging::logf(logging::log_level::TRACE, "tcp: Target Port %h \n", target_port);

    packet.index += sizeof(header);

    //TODO
}

std::expected<network::ethernet::packet> network::tcp::prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size){
    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(interface, sizeof(header) + payload_size, target_ip, 0x11);

    if(packet){
        ::prepare_packet(*packet, source, target, payload_size);
    }

    return packet;
}

std::expected<network::ethernet::packet> network::tcp::prepare_packet(char* buffer, network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size){
    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(buffer, interface, sizeof(header) + payload_size, target_ip, 0x11);

    if(packet){
        ::prepare_packet(*packet, source, target, payload_size);
    }

    return packet;
}

void network::tcp::finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p){
    p.index -= sizeof(header);

    // Compute the checksum
    compute_checksum(p);

    // Give the packet to the IP layer for finalization
    network::ip::finalize_packet(interface, p);
}

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <bit_field.hpp>

#include "net/tcp_layer.hpp"
#include "net/dns_layer.hpp"
#include "net/checksum.hpp"

#include "kernel_utils.hpp"

namespace {

using flag_data_offset = std::bit_field<uint16_t, uint8_t, 12, 4>;
using flag_reserved    = std::bit_field<uint16_t, uint8_t, 9, 3>;
using flag_ns          = std::bit_field<uint16_t, uint8_t, 8, 1>;
using flag_cwr         = std::bit_field<uint16_t, uint8_t, 7, 1>;
using flag_ece         = std::bit_field<uint16_t, uint8_t, 6, 1>;
using flag_urg         = std::bit_field<uint16_t, uint8_t, 5, 1>;
using flag_ack         = std::bit_field<uint16_t, uint8_t, 4, 1>;
using flag_psh         = std::bit_field<uint16_t, uint8_t, 3, 1>;
using flag_rst         = std::bit_field<uint16_t, uint8_t, 2, 1>;
using flag_syn         = std::bit_field<uint16_t, uint8_t, 1, 1>;
using flag_fin         = std::bit_field<uint16_t, uint8_t, 0, 1>;

void compute_checksum(network::ethernet::packet& packet){
    auto* ip_header  = reinterpret_cast<network::ip::header*>(packet.payload + packet.tag(1));
    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet.payload + packet.index);

    tcp_header->checksum = 0;

    // Accumulate the Payload
    auto sum = network::checksum_add_bytes(packet.payload + packet.index, 20); // TODO What is the length

    // Accumulate the IP addresses
    sum += network::checksum_add_bytes(&ip_header->source_ip, 8);

    // Accumulate the IP Protocol
    sum += ip_header->protocol;

    // Accumulate the UDP length
    sum += 20;

    // Complete the 1-complement sum
    tcp_header->checksum = switch_endian_16(network::checksum_finalize_nz(sum));
}

uint16_t get_default_flags(){
    uint16_t flags = 0; // By default

    (flag_data_offset(&flags)) = 5; // No options

    //TODO

    return flags;
}

void prepare_packet(network::ethernet::packet& packet, size_t source, size_t target, size_t payload_size){
    packet.tag(2, packet.index);

    // Set the TCP header

    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet.payload + packet.index);

    tcp_header->source_port = switch_endian_16(source);
    tcp_header->target_port = switch_endian_16(target);
    tcp_header->window_size = 1024;

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
    auto packet = network::ip::prepare_packet(interface, sizeof(header) + payload_size, target_ip, 0x06);

    if(packet){
        ::prepare_packet(*packet, source, target, payload_size);
    }

    return packet;
}

std::expected<network::ethernet::packet> network::tcp::prepare_packet(char* buffer, network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size){
    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(buffer, interface, sizeof(header) + payload_size, target_ip, 0x06);

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

std::expected<void> network::tcp::connect(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target){
    auto packet = tcp::prepare_packet(interface, target_ip, source, target, 0);

    if(!packet){
        return std::make_unexpected<void>(packet.error());
    }

    auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

    tcp_header->sequence_number = 0;
    tcp_header->ack_number      = 0;

    auto flags = get_default_flags();
    (flag_syn(&flags)) = 1;
    tcp_header->flags = switch_endian_16(flags);

    tcp::finalize_packet(interface, *packet);

    return {};
}

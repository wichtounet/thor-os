//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/udp_layer.hpp"

#include "kernel_utils.hpp"

namespace {

void compute_checksum(network::ethernet::packet& packet, network::udp::header* udp_header){
    auto ip_index = packet.tag(1);

    auto* ip_header = reinterpret_cast<network::ip::header*>(packet.payload + ip_index);

    udp_header->checksum = 0;

    // Accumulate the ICMP header
    auto sum = std::accumulate(
        reinterpret_cast<uint16_t*>(udp_header),
        reinterpret_cast<uint16_t*>(udp_header) + udp_header->length * 2,
        uint32_t(0));

    // Accumulate the IP addresses
    sum = std::accumulate(
        reinterpret_cast<uint16_t*>(&ip_header->source_ip),
        reinterpret_cast<uint16_t*>(&ip_header->source_ip) + 4,
        sum);

    // Accumulate the IP Protocol
    sum += ip_header->protocol;

    // Accumulate the UDP length
    sum += udp_header->length;

    // Complete the 1-complement sum

    uint32_t value = sum & 0xFFFF;
    uint32_t carry = (sum - value) >> 16;

    while(carry){
        value += carry;
        auto sub = value & 0xFFFF;
        carry = (value - sub) >> 16;
        value = sub;
    }

    udp_header->checksum = ~value;

    if(!udp_header->checksum){
        udp_header->checksum = ~0;
    }
}

void prepare_packet(network::ethernet::packet& packet, size_t source, size_t target, size_t payload_size){
    // Set the UDP header

    auto* udp_header = reinterpret_cast<network::udp::header*>(packet.payload + packet.index);

    udp_header->source_port = switch_endian_16(source);
    udp_header->target_port = switch_endian_16(target);
    udp_header->length      = switch_endian_16(sizeof(network::udp::header) + payload_size);

    packet.index += sizeof(network::udp::header);
}

} //end of anonymous namespace

void network::udp::decode(network::interface_descriptor& /*interface*/, network::ethernet::packet& packet){
    packet.tag(2, packet.index);

    auto* udp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "udp: Start UDP packet handling\n");

    auto source_port = switch_endian_16(udp_header->source_port);
    auto target_port = switch_endian_16(udp_header->target_port);
    auto length      = switch_endian_16(udp_header->length);

    logging::logf(logging::log_level::TRACE, "udp: Source Port %h \n", source_port);
    logging::logf(logging::log_level::TRACE, "udp: Target Port %h \n", target_port);
    logging::logf(logging::log_level::TRACE, "udp: Length %h \n", length);

    if(target_port == 53){
        //TODO DNS decoding
    }
}

std::expected<network::ethernet::packet> network::udp::prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size){
    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(interface, sizeof(header) + payload_size, target_ip, 0x11);

    if(packet){
        ::prepare_packet(*packet, source, target, payload_size);
    }

    return packet;
}

std::expected<network::ethernet::packet> network::udp::prepare_packet(char* buffer, network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size){
    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(buffer, interface, sizeof(header) + payload_size, target_ip, 0x11);

    if(packet){
        ::prepare_packet(*packet, source, target, payload_size);
    }

    return packet;
}

void network::udp::finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p){
    p.index -= sizeof(header);

    auto* udp_header = reinterpret_cast<header*>(p.payload + p.index);

    // Compute the checksum
    compute_checksum(p, udp_header);

    // Give the packet to the IP layer for finalization
    network::ip::finalize_packet(interface, p);
}

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/udp_layer.hpp"
#include "net/dns_layer.hpp"

#include "kernel_utils.hpp"

namespace {

template<typename T>
uint32_t net_checksum_add_bytes(T* values, size_t length){
    auto raw_values = reinterpret_cast<uint8_t*>(values);

    uint32_t sum = 0;

    for(size_t i = 0; i < length; ++i){
        if(i & 1){
            sum += static_cast<uint32_t>(raw_values[i]);
        } else {
            sum += static_cast<uint32_t>(raw_values[i]) << 8;
        }
    }

    return sum;
}

uint16_t net_checksum_finalize(uint32_t sum){
    while(sum >> 16){
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

uint16_t net_checksum_finalize_nz(uint32_t sum){
    auto checksum = net_checksum_finalize(sum);

    if(!checksum){
        return ~checksum;
    } else {
        return checksum;
    }
}

void compute_checksum(network::ethernet::packet& packet){
    auto* ip_header = reinterpret_cast<network::ip::header*>(packet.payload + packet.tag(1));
    auto* udp_header = reinterpret_cast<network::udp::header*>(packet.payload + packet.index);

    udp_header->checksum = 0;

    auto length = switch_endian_16(udp_header->length);

    // Accumulate the Payload
    auto sum = net_checksum_add_bytes(packet.payload + packet.index, length);

    // Accumulate the IP addresses
    sum += net_checksum_add_bytes(&ip_header->source_ip, 8);

    // Accumulate the IP Protocol
    sum += ip_header->protocol;

    // Accumulate the UDP length
    sum += length;

    // Complete the 1-complement sum
    udp_header->checksum = switch_endian_16(net_checksum_finalize_nz(sum));
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

    // Compute the checksum
    compute_checksum(p);

    // Give the packet to the IP layer for finalization
    network::ip::finalize_packet(interface, p);
}

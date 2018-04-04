//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <string.hpp>

#include "net/ip_layer.hpp"
#include "net/ethernet_layer.hpp"
#include "net/icmp_layer.hpp"
#include "net/udp_layer.hpp"
#include "net/tcp_layer.hpp"
#include "net/arp_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"

namespace {

constexpr const size_t ARP_TIMEOUT = 5000;
constexpr size_t default_ip_header_len = 20;

void compute_checksum(network::ip::header* header){
    auto ihl = header->version_ihl & 0xF;

    header->header_checksum = 0;

    auto sum = std::accumulate(reinterpret_cast<uint16_t*>(header),reinterpret_cast<uint16_t*>(header) + ihl * 2, uint32_t(0));

    uint32_t value = sum & 0xFFFF;
    uint32_t carry = (sum - value) >> 16;

    while(carry){
        value += carry;
        auto sub = value & 0xFFFF;
        carry = (value - sub) >> 16;
        value = sub;
    }

    header->header_checksum = ~uint16_t(value);
}

void prepare_packet(network::packet& packet, network::interface_descriptor& interface, size_t size, network::ip::address target_ip, size_t protocol){
    packet.tag(1, packet.index);

    auto* ip_header = reinterpret_cast<network::ip::header*>(packet.payload + packet.index);

    ip_header->version_ihl    = (4 << 4) + default_ip_header_len / 4;
    ip_header->dscp_ecn       = 0;
    ip_header->total_len      = switch_endian_16(uint16_t(size) + default_ip_header_len);
    ip_header->identification = 0;
    ip_header->flags_offset   = 0;
    ip_header->flags_offset   = switch_endian_16(uint16_t(1) << 14);
    ip_header->ttl            = 255;
    ip_header->protocol       = protocol;
    ip_header->source_ip      = ip_to_ip32(interface.ip_address);
    ip_header->target_ip      = ip_to_ip32(target_ip);

    compute_checksum(ip_header);

    packet.index += default_ip_header_len;
}

} // end of anonymous namespace

network::ip::address network::ip::ip32_to_ip(uint32_t raw){
    return {switch_endian_32(raw)};
}

uint32_t network::ip::ip_to_ip32(address ip){
    return switch_endian_32(ip.raw_address);
}

std::string network::ip::ip_to_str(address ip){
    std::string value;
    value += std::to_string(ip(0));
    value += '.';
    value += std::to_string(ip(1));
    value += '.';
    value += std::to_string(ip(2));
    value += '.';
    value += std::to_string(ip(3));
    return value;
}

bool network::ip::same_network(address ip, address test) {
    return ip(0) == test(0) && ip(1) == test(1) && ip(2) == test(2);
}

network::ip::layer::layer(network::ethernet::layer* parent) : parent(parent) {
    parent->register_ip_layer(this);
}

void network::ip::layer::decode(network::interface_descriptor& interface, network::packet_p& packet){
    logging::logf(logging::log_level::TRACE, "ip: Start IPv4 packet handling (%p)\n", packet.get());

    packet->tag(1, packet->index);

    auto* ip_header = reinterpret_cast<header*>(packet->payload + packet->index);

    auto version = ip_header->version_ihl >> 4;

    if(version != 4){
        logging::logf(logging::log_level::ERROR, "ip: IPv6 Packet received instead of IPv4\n");

        return;
    }

    const auto header_length = 4 * (ip_header->version_ihl & 0xF);
    auto length              = switch_endian_16(ip_header->total_len);
    auto data_length         = length - header_length;

    logging::logf(logging::log_level::TRACE, "ip: Data Length: %u\n", size_t(data_length));
    logging::logf(logging::log_level::TRACE, "ip: Time To Live: %u\n", size_t(ip_header->ttl));

    auto source = ip32_to_ip(ip_header->source_ip);
    auto target = ip32_to_ip(ip_header->target_ip);

    logging::logf(logging::log_level::TRACE, "ip: Source Protocol Address %u.%u.%u.%u \n",
                  uint64_t(source(0)), uint64_t(source(1)), uint64_t(source(2)), uint64_t(source(3)));
    logging::logf(logging::log_level::TRACE, "ip: Target Protocol Address %u.%u.%u.%u \n",
                  uint64_t(target(0)), uint64_t(target(1)), uint64_t(target(2)), uint64_t(target(3)));

    auto protocol = ip_header->protocol;

    packet->index += header_length;

    if (protocol == 0x01) {
        icmp_layer->decode(interface, packet);
    } else if (protocol == 0x06) {
        tcp_layer->decode(interface, packet);
    } else if (protocol == 0x11) {
        udp_layer->decode(interface, packet);
    } else {
        logging::logf(logging::log_level::ERROR, "ip: Packet of unknown protocol detected (%h)\n", size_t(protocol));
    }
}

std::expected<network::packet_p> network::ip::layer::kernel_prepare_packet(network::interface_descriptor& interface, const packet_descriptor& descriptor){
    auto target_mac = get_target_mac(interface, descriptor.destination);

    if(!target_mac){
        return std::make_expected_from_error<network::packet_p>(target_mac.error());
    }

    // Ask the ethernet layer to craft a packet
    network::ethernet::packet_descriptor desc{descriptor.size + default_ip_header_len, *target_mac, ethernet::ether_type::IPV4};
    auto packet = parent->kernel_prepare_packet(interface, desc);

    if(packet){
        ::prepare_packet(**packet, interface, descriptor.size, descriptor.destination, descriptor.protocol);
    }

    return packet;
}

std::expected<network::packet_p> network::ip::layer::user_prepare_packet(char* buffer, network::interface_descriptor& interface, const packet_descriptor* descriptor){
    auto target_mac = get_target_mac(interface, descriptor->destination);

    if(!target_mac){
        return std::make_expected_from_error<network::packet_p>(target_mac.error());
    }

    // Ask the ethernet layer to craft a packet
    network::ethernet::packet_descriptor desc{descriptor->size + default_ip_header_len, *target_mac, ethernet::ether_type::IPV4};
    auto packet = parent->user_prepare_packet(buffer, interface, &desc);

    if(packet){
        ::prepare_packet(**packet, interface, descriptor->size, descriptor->destination, descriptor->protocol);
    }

    return packet;
}

std::expected<void> network::ip::layer::finalize_packet(network::interface_descriptor& interface, network::packet_p& p){
    // Send the packet to the ethernet layer
    return parent->finalize_packet(interface, p);
}

std::expected<uint64_t> network::ip::layer::get_target_mac(network::interface_descriptor& interface, network::ip::address target_ip){
    // Handle broadcast
    if(target_ip == network::ip::make_address(255, 255, 255, 255)){
        return 0xFFFFFFFFFFFF;
    }

    // For loopback, there is no gateway
    if(interface.is_loopback()){
        return arp_layer->get_cache().get_mac_force(interface, target_ip, ARP_TIMEOUT);
    }

    auto& interface_ip = interface.ip_address;

    // At this point, we have no gateway, neither IP
    if(interface_ip == network::ip::make_address(0, 0, 0, 0)){
        return arp_layer->get_cache().get_mac_force(interface, target_ip, ARP_TIMEOUT);
    }

    // If it is the same network, use ARP to get the MAC address
    if(network::ip::same_network(interface_ip, target_ip)){
        return arp_layer->get_cache().get_mac_force(interface, target_ip, ARP_TIMEOUT);
    }

    // If it is another network, use the gateway

    return arp_layer->get_cache().get_mac_force(interface, interface.gateway, ARP_TIMEOUT);
}
void network::ip::layer::register_icmp_layer(network::icmp::layer* layer){
    this->icmp_layer = layer;
}

void network::ip::layer::register_arp_layer(network::arp::layer* layer){
    this->arp_layer = layer;
}

void network::ip::layer::register_udp_layer(network::udp::layer* layer){
    this->udp_layer = layer;
}

void network::ip::layer::register_tcp_layer(network::tcp::layer* layer){
    this->tcp_layer = layer;
}

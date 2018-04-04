//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/ethernet_layer.hpp"
#include "net/arp_layer.hpp"
#include "net/ip_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"

namespace {

network::ethernet::ether_type decode_ether_type(network::ethernet::header* header){
    auto type = switch_endian_16(header->type);

    if(type == 0x800){
        return network::ethernet::ether_type::IPV4;
    } else if(type == 0x86DD){
        return network::ethernet::ether_type::IPV6;
    } else if(type == 0x806){
        return network::ethernet::ether_type::ARP;
    } else {
        return network::ethernet::ether_type::UNKNOWN;
    }
}

uint16_t type_to_code(network::ethernet::ether_type type){
    switch(type){
        case network::ethernet::ether_type::IPV4:
            return 0x800;
        case network::ethernet::ether_type::IPV6:
            return 0x86DD;
        case network::ethernet::ether_type::ARP:
            return 0x806;
        case network::ethernet::ether_type::UNKNOWN:
            logging::logf(logging::log_level::ERROR, "ethernet: Decoding UNKNOWN code\n");
            return 0x0;
    }

    logging::logf(logging::log_level::ERROR, "ethernet: Decoding UNKNOWN code\n");
    return 0x0;
}

void prepare_packet(network::packet& p, network::interface_descriptor& interface, const network::ethernet::packet_descriptor& descriptor){
    p.tag(0, 0);
    p.index = sizeof(network::ethernet::header);
    p.interface = interface.id;

    auto source_mac = interface.mac_address;

    auto* ether_header = reinterpret_cast<network::ethernet::header*>(p.payload);
    ether_header->type = switch_endian_16(type_to_code(descriptor.type));

    network::ethernet::mac64_to_mac6(source_mac, ether_header->source.mac);
    network::ethernet::mac64_to_mac6(descriptor.destination, ether_header->target.mac);
}

} //end of anonymous namespace

uint64_t network::ethernet::mac6_to_mac64(const char* source_mac){
    size_t mac = 0;

    for(size_t i = 0; i < 6; ++i){
        mac |= uint64_t(source_mac[i]) << ((5 - i) * 8);
    }

    return mac;
}

void network::ethernet::mac64_to_mac6(uint64_t source_mac, char* mac){
    for(size_t i = 0; i < 6; ++i){
        mac[i] = (source_mac >> ((5 - i) * 8));
    }
}

void network::ethernet::layer::decode(network::interface_descriptor& interface, packet_p& packet){
    logging::logf(logging::log_level::TRACE, "ethernet: Start decoding new packet (%p)\n", packet.get());

    auto* ether_header = reinterpret_cast<header*>(packet->payload);

    // Filter out non-ethernet II frames
    if(switch_endian_16(ether_header->type) < 1536){
        logging::logf(logging::log_level::ERROR, "ethernet: error only ethernet frame type II is supported\n");
        return;
    }

    size_t source_mac = mac6_to_mac64(ether_header->source.mac);
    size_t target_mac = mac6_to_mac64(ether_header->target.mac);

    logging::logf(logging::log_level::TRACE, "ethernet: Source MAC Address %h \n", source_mac);
    logging::logf(logging::log_level::TRACE, "ethernet: Destination MAC Address %h \n", target_mac);

    packet->tag(0, 0);
    packet->index = sizeof(header);

    auto type = decode_ether_type(ether_header);

    switch (type) {
        case ether_type::IPV4:
            ip_layer->decode(interface, packet);
            break;

        case ether_type::ARP:
            arp_layer->decode(interface, packet);
            break;

        case ether_type::IPV6:
            logging::logf(logging::log_level::TRACE, "ethernet: IPV6 Packet (unsupported)\n");
            break;

        case ether_type::UNKNOWN:
            logging::logf(logging::log_level::TRACE, "ethernet: Unhandled Packet Type: %u\n", uint64_t(switch_endian_16(ether_header->type)));
            break;

        default:
            logging::logf(logging::log_level::ERROR, "ethernet: Unhandled Packet Type in switch: %u\n", uint64_t(switch_endian_16(ether_header->type)));
            break;
    }

    logging::logf(logging::log_level::TRACE, "ethernet: Finished decoding packet\n");
}

std::expected<network::packet_p> network::ethernet::layer::kernel_prepare_packet(network::interface_descriptor& interface, const packet_descriptor& descriptor){
    auto total_size = descriptor.size + sizeof(header);

    auto p = std::make_shared<network::packet>(new char[total_size], total_size);

    ::prepare_packet(*p, interface, descriptor);

    return p;
}

std::expected<network::packet_p> network::ethernet::layer::user_prepare_packet(char* buffer, network::interface_descriptor& interface, const packet_descriptor* descriptor){
    auto total_size = descriptor->size + sizeof(header);

    auto p = std::make_shared<network::packet>(buffer, total_size);
    p->user = true;

    ::prepare_packet(*p, interface, *descriptor);

    return p;
}

std::expected<void> network::ethernet::layer::finalize_packet(network::interface_descriptor& interface, packet_p& p){
    if(p->user){
        // The packet will be handled by a kernel thread, needs to
        // be copied to kernel memory

        auto kernel_packet = std::make_shared<network::packet>(new char[p->payload_size], p->payload_size);
        std::copy_n(p->payload, p->payload_size, kernel_packet->payload);

        interface.send(kernel_packet);
    } else {
        interface.send(p);
    }

    return {};
}

void network::ethernet::layer::register_arp_layer(network::arp::layer* layer){
    this->arp_layer = layer;
}

void network::ethernet::layer::layer::register_ip_layer(network::ip::layer* layer){
    this->ip_layer = layer;
}

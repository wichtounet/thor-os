//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/errors.hpp"

#include "net/icmp_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/network.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"
#include "scheduler.hpp"

namespace {

void compute_checksum(network::icmp::header* icmp_header, size_t payload_size){
    icmp_header->checksum = 0;

    auto sum = std::accumulate(
        reinterpret_cast<uint16_t*>(icmp_header),
        reinterpret_cast<uint16_t*>(icmp_header) + sizeof(network::icmp::header) / 2 + payload_size * 2,
        uint32_t(0));

    uint32_t value = sum & 0xFFFF;
    uint32_t carry = (sum - value) >> 16;

    while(carry){
        value += carry;
        auto sub = value & 0xFFFF;
        carry = (value - sub) >> 16;
        value = sub;
    }

    icmp_header->checksum = ~value;
}

void prepare_packet(network::packet& packet, network::icmp::type t, size_t code){
    packet.tag(2, packet.index);

    // Set the ICMP header

    auto* icmp_header = reinterpret_cast<network::icmp::header*>(packet.payload + packet.index);

    icmp_header->type = static_cast<uint8_t>(t);
    icmp_header->code = code;

    packet.index += sizeof(network::icmp::header) - sizeof(uint32_t);
}

} // end of anonymous namespace

network::icmp::layer::layer(network::ip::layer* parent) : parent(parent) {
    parent->register_icmp_layer(this);
}

void network::icmp::layer::decode(network::interface_descriptor& interface, network::packet_p& packet){
    logging::logf(logging::log_level::TRACE, "icmp: Start ICMP packet handling (%p)\n", packet.get());

    packet->tag(2, packet->index);

    auto* icmp_header = reinterpret_cast<header*>(packet->payload + packet->index);

    auto command_type = static_cast<type>(icmp_header->type);

    auto command_index = packet->index + sizeof(network::icmp::header) - sizeof(uint32_t);

    switch(command_type){
        case type::ECHO_REQUEST:
            {
                logging::logf(logging::log_level::TRACE, "icmp: received Echo Request\n");

                auto ip_index = packet->tag(1);
                auto* ip_header = reinterpret_cast<network::ip::header*>(packet->payload + ip_index);

                auto target_ip = network::ip::ip32_to_ip(ip_header->target_ip);
                auto source_ip = network::ip::ip32_to_ip(ip_header->source_ip);

                if(target_ip == interface.ip_address){
                    logging::logf(logging::log_level::TRACE, "icmp: Reply to Echo Request for own IP\n");

                    network::icmp::packet_descriptor desc{0, source_ip, type::ECHO_REPLY, 0x0};
                    auto reply_packet_e = kernel_prepare_packet(interface, desc);

                    if(reply_packet_e){
                        auto& reply_packet = *reply_packet_e;

                        auto* command_header = reinterpret_cast<echo_request_header*>(packet->payload + command_index);
                        auto* reply_command_header = reinterpret_cast<echo_request_header*>(reply_packet->payload + reply_packet->index);

                        *reply_command_header = *command_header;

                        finalize_packet(interface, reply_packet);
                    } else {
                        logging::logf(logging::log_level::ERROR, "icmp: Failed to reply: %s\n", std::error_message(reply_packet_e.error()));
                    }
                }

                break;
            }
        case type::ECHO_REPLY:
            logging::logf(logging::log_level::TRACE, "icmp: Echo Reply\n");
            break;
        case type::UNREACHABLE:
            logging::logf(logging::log_level::TRACE, "icmp: Unreachable\n");
            break;
        case type::TIME_EXCEEDED:
            logging::logf(logging::log_level::TRACE, "icmp: Time exceeded\n");
            break;
        default:
            logging::logf(logging::log_level::TRACE, "icmp: Unsupported ICMP packet received (type:%u)\n", uint64_t(icmp_header->type));
            break;
    }

    logging::logf(logging::log_level::TRACE, "icmp: Propagate (%p)\n", packet.get());

    network::propagate_packet(packet, network::socket_protocol::ICMP);

    logging::logf(logging::log_level::TRACE, "icmp: Finished packet handling (%p)\n", packet.get());
}

std::expected<network::packet_p> network::icmp::layer::kernel_prepare_packet(network::interface_descriptor& interface, const packet_descriptor& descriptor){
    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{sizeof(header) + descriptor.payload_size, descriptor.target_ip, 0x01};
    auto packet = parent->kernel_prepare_packet(interface, desc);

    if(packet){
        ::prepare_packet(**packet, descriptor.type, descriptor.code);
    }

    return packet;
}

std::expected<network::packet_p> network::icmp::layer::user_prepare_packet(char* buffer, network::socket& /*socket*/, const packet_descriptor* descriptor){
    auto& interface = network::select_interface(descriptor->target_ip);

    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{sizeof(header) + descriptor->payload_size, descriptor->target_ip, 0x01};
    auto packet = parent->user_prepare_packet(buffer, interface, &desc);

    if(packet){
        ::prepare_packet(**packet, descriptor->type, descriptor->code);
    }

    return packet;
}

std::expected<void> network::icmp::layer::finalize_packet(network::interface_descriptor& interface, network::packet_p& packet){
    packet->index -= sizeof(header) - sizeof(uint32_t);

    auto* icmp_header = reinterpret_cast<header*>(packet->payload + packet->index);

    // Compute the checksum
    compute_checksum(icmp_header, 0);

    // Give the packet to the IP layer for finalization
    return parent->finalize_packet(interface, packet);
}

std::expected<void> network::icmp::layer::finalize_packet(network::interface_descriptor& interface, network::socket& /*sock*/, network::packet_p& packet){
    return this->finalize_packet(interface, packet);
}

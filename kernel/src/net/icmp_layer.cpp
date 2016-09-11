//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/ethernet_layer.hpp"
#include "net/icmp_layer.hpp"
#include "net/arp_cache.hpp"
#include "net/ip_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"
#include "scheduler.hpp"

#include "tlib/errors.hpp"

namespace {

uint16_t echo_sequence = 0;

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

void prepare_packet(network::ethernet::packet& packet, network::icmp::type t, size_t code){
    packet.tag(2, packet.index);

    // Set the ICMP header

    auto* icmp_header = reinterpret_cast<network::icmp::header*>(packet.payload + packet.index);

    icmp_header->type = static_cast<uint8_t>(t);
    icmp_header->code = code;

    packet.index += sizeof(network::icmp::header) - sizeof(uint32_t);
}

} // end of anonymous namespace

void network::icmp::decode(network::interface_descriptor& interface, network::ethernet::packet& packet){
    packet.tag(2, packet.index);

    logging::logf(logging::log_level::TRACE, "icmp: Start ICMP packet handling\n");

    auto* icmp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    auto command_type = static_cast<type>(icmp_header->type);

    auto command_index = packet.index + sizeof(network::icmp::header) - sizeof(uint32_t);

    switch(command_type){
        case type::ECHO_REQUEST:
            {
                logging::logf(logging::log_level::TRACE, "icmp: received Echo Request\n");

                auto ip_index = packet.tag(1);
                auto* ip_header = reinterpret_cast<network::ip::header*>(packet.payload + ip_index);

                auto target_ip = network::ip::ip32_to_ip(ip_header->target_ip);
                auto source_ip = network::ip::ip32_to_ip(ip_header->source_ip);

                if(target_ip == interface.ip_address){
                    logging::logf(logging::log_level::TRACE, "icmp: Reply to Echo Request for own IP\n");

                    auto reply_packet = network::icmp::prepare_packet(interface, source_ip, 0x0, type::ECHO_REPLY, 0x0);

                    if(reply_packet){
                        auto* command_header = reinterpret_cast<echo_request_header*>(packet.payload + command_index);
                        auto* reply_command_header = reinterpret_cast<echo_request_header*>(reply_packet->payload + reply_packet->index);

                        *reply_command_header = *command_header;
                    } else {
                        logging::logf(logging::log_level::ERROR, "icmp: Failed to reply: %s\n", std::error_message(reply_packet.error()));
                    }

                    network::icmp::finalize_packet(interface, *reply_packet);
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

    // TODO Need something better for this

    for(size_t pid = 0; pid < scheduler::MAX_PROCESS; ++pid){
        auto state = scheduler::get_process_state(pid);
        if(state != scheduler::process_state::EMPTY && state != scheduler::process_state::NEW && state != scheduler::process_state::KILLED){
            for(auto& socket : scheduler::get_sockets(pid)){
                if(socket.listen && socket.protocol == network::socket_protocol::ICMP){
                    auto copy = packet;
                    copy.payload = new char[copy.payload_size];
                    std::copy_n(packet.payload, packet.payload_size, copy.payload);

                    socket.listen_packets.push(copy);
                    socket.listen_queue.wake_up();
                }
            }
        }
    }
}

std::expected<network::ethernet::packet> network::icmp::prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t payload_size, type t, size_t code){
    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(interface, sizeof(header) + payload_size, target_ip, 0x01);

    if(packet){
        ::prepare_packet(*packet, t, code);
    }

    return packet;
}

std::expected<network::ethernet::packet> network::icmp::prepare_packet(char* buffer, network::interface_descriptor& interface, network::ip::address target_ip, size_t payload_size, type t, size_t code){
    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(buffer, interface, sizeof(header) + payload_size, target_ip, 0x01);

    if(packet){
        ::prepare_packet(*packet, t, code);
    }

    return packet;
}

void network::icmp::finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& packet){
    packet.index -= sizeof(header) - sizeof(uint32_t);

    auto* icmp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    // Compute the checksum
    compute_checksum(icmp_header, 0);

    // Give the packet to the IP layer for finalization
    network::ip::finalize_packet(interface, packet);
}

void network::icmp::ping(network::interface_descriptor& interface, network::ip::address target_ip){
    logging::logf(logging::log_level::TRACE, "icmp: Ping %u.%u.%u.%u \n",
        uint64_t(target_ip(0)), uint64_t(target_ip(1)), uint64_t(target_ip(2)), uint64_t(target_ip(3)));

    auto target_mac = network::arp::get_mac_force(interface, target_ip);

    if(!target_mac){
        logging::logf(logging::log_level::TRACE, "icmp: Failed to get MAC Address from IP\n");
        return;
    }

    logging::logf(logging::log_level::TRACE, "icmp: Target MAC Address: %h\n", *target_mac);

    // Ask the ICMP layer to craft a packet
    auto packet = network::icmp::prepare_packet(interface, target_ip, 0, type::ECHO_REQUEST, 0);

    if(packet){
        // Set the Command header

        auto* command_header = reinterpret_cast<echo_request_header*>(packet->payload + packet->index);

        command_header->identifier = 0x666;
        command_header->sequence = echo_sequence++;

        // Send the packet back to ICMP
        network::icmp::finalize_packet(interface, *packet);
    }
}

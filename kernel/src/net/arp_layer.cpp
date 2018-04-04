//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/errors.hpp"

#include "net/arp_layer.hpp"
#include "net/ethernet_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"

uint64_t network::arp::mac3_to_mac64(uint16_t* source_mac){
    size_t mac = 0;

    for(size_t i = 0; i < 3; ++i){
        mac |= uint64_t(switch_endian_16(source_mac[i])) << ((2 - i) * 16);
    }

    return mac;
}

network::ip::address network::arp::ip2_to_ip(uint16_t* source_ip){
    network::ip::address ip;

    for(size_t i = 0; i < 2; ++i){
        auto source = switch_endian_16(source_ip[i]);

        ip.set_sub(2*i, source >> 8);
        ip.set_sub(2*i+1, source);
    }

    return ip;
}

void network::arp::mac64_to_mac3(uint64_t source_mac, uint16_t* mac){
    for(size_t i = 0; i < 3; ++i){
        mac[i] = switch_endian_16(uint16_t(source_mac >> ((2 - i) * 16)));
    }
}

void network::arp::ip_to_ip2(network::ip::address source_ip, uint16_t* ip){
    for(size_t i = 0; i < 2; ++i){
        ip[i] = (uint16_t(source_ip(2*i+1)) << 8) + source_ip(2*i);
    }
}

network::arp::layer::layer(network::ethernet::layer* parent) : parent(parent), _cache(this, parent) {
    parent->register_arp_layer(this);
}

void network::arp::layer::decode(network::interface_descriptor& interface, network::packet_p& packet){
    packet->tag(1, packet->index);

    auto* arp_header = reinterpret_cast<header*>(packet->payload + packet->index);

    logging::logf(logging::log_level::TRACE, "arp: Start ARP packet handling\n");

    auto hw_type = switch_endian_16(arp_header->hw_type);
    auto protocol_type = switch_endian_16(arp_header->protocol_type);

    // We only support ethernet hardware frames
    if(hw_type != 0x1){
        logging::logf(logging::log_level::ERROR, "arp: Unhandled hardware type %h\n", size_t(hw_type));
        return;
    }

    // We only support IPV4 protocol
    if(protocol_type != 0x800){
        logging::logf(logging::log_level::ERROR, "arp: Unhandled protocol type %h\n", size_t(protocol_type));
        return;
    }

    auto hw_len = arp_header->hw_len;
    auto protocol_len = arp_header->protocol_len;
    auto operation = switch_endian_16(arp_header->operation);

    // This should never happen
    if(hw_len != 0x6 || protocol_len != 0x4){
        logging::logf(logging::log_level::ERROR, "arp: Unable to process the given length hw:%h prot:%h\n", size_t(hw_len), size_t(protocol_len));
        return;
    }

    if (operation != 0x1 && operation != 0x2) {
        logging::logf(logging::log_level::TRACE, "arp: Unhandled operation %h\n", size_t(operation));
        return;
    }

    auto source_hw = mac3_to_mac64(arp_header->source_hw_addr);
    auto target_hw = mac3_to_mac64(arp_header->target_hw_addr);

    logging::logf(logging::log_level::TRACE, "arp: Source HW Address %h \n", source_hw);
    logging::logf(logging::log_level::TRACE, "arp: Target HW Address %h \n", target_hw);

    auto source_prot = ip2_to_ip(arp_header->source_protocol_addr);
    auto target_prot = ip2_to_ip(arp_header->target_protocol_addr);

    logging::logf(logging::log_level::TRACE, "arp: Source Protocol Address %u.%u.%u.%u \n",
        uint64_t(source_prot(0)), uint64_t(source_prot(1)), uint64_t(source_prot(2)), uint64_t(source_prot(3)));
    logging::logf(logging::log_level::TRACE, "arp: Target Protocol Address %u.%u.%u.%u \n",
        uint64_t(target_prot(0)), uint64_t(target_prot(1)), uint64_t(target_prot(2)), uint64_t(target_prot(3)));

    // If not an ARP Probe, update the ARP cache
    if(source_prot.raw_address != 0x0){
        _cache.update_cache(source_hw, source_prot);
    }

    if(operation == 0x1){
        if(target_prot == interface.ip_address){
            logging::logf(logging::log_level::TRACE, "arp: Reply to Request for own IP\n");

            // Ask the ethernet layer to craft a packet
            network::ethernet::packet_descriptor desc{sizeof(header), source_hw, ethernet::ether_type::ARP};
            auto packet_e = parent->kernel_prepare_packet(interface, desc);

            if(packet_e){
                auto& packet = *packet_e;

                auto* arp_reply_header = reinterpret_cast<header*>(packet->payload + packet->index);

                arp_reply_header->hw_type = switch_endian_16(0x1); // ethernet
                arp_reply_header->protocol_type = switch_endian_16(0x800); // IPV4
                arp_reply_header->hw_len = 0x6; // MAC Address
                arp_reply_header->protocol_len = 0x4; // IP Address
                arp_reply_header->operation = switch_endian_16(0x2); //ARP Reply

                mac64_to_mac3(interface.mac_address, arp_reply_header->source_hw_addr);

                std::copy_n(arp_header->source_hw_addr, 3, arp_reply_header->target_hw_addr);

                std::copy_n(arp_header->target_protocol_addr, 2, arp_reply_header->source_protocol_addr);
                std::copy_n(arp_header->source_protocol_addr, 2, arp_reply_header->target_protocol_addr);

                parent->finalize_packet(interface, packet);
            } else {
                logging::logf(logging::log_level::ERROR, "arp: Impossible to reply to ARP Request: %s\n", std::error_message(packet_e.error()));

            }
        }
    } else if(operation == 0x2){
        logging::logf(logging::log_level::TRACE, "arp: Handle Reply\n");

        wait_queue.notify_all();
    }
}

void network::arp::layer::wait_for_reply(){
    wait_queue.wait();
}

void network::arp::layer::wait_for_reply(size_t ms){
    wait_queue.wait_for(ms);
}

network::arp::cache& network::arp::layer::get_cache(){
    return _cache;
}

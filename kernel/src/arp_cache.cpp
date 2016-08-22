//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "arp_cache.hpp"
#include "arp_layer.hpp"
#include "logging.hpp"
#include "kernel_utils.hpp"
#include "assert.hpp"

namespace {

struct cache_entry {
    uint64_t mac;
    network::ip::address ip;

    cache_entry(){}
    cache_entry(uint64_t mac, network::ip::address ip) : mac(mac), ip(ip) {}
};

std::vector<cache_entry> cache;

} //end of anonymous namespace

void network::arp::update_cache(uint64_t mac, network::ip::address ip){
    for(auto& entry : cache){
        if(entry.mac == mac && entry.ip == ip){
            return;
        } else if(entry.mac == mac || entry.ip == ip){
            logging::logf(logging::log_level::TRACE, "arp: Update cache %h->%u.%u.%u.%u \n",
                mac, ip(0), ip(1), ip(2), ip(3));

            entry.mac = mac;
            entry.ip = ip;
            return;
        }
    }

    logging::logf(logging::log_level::TRACE, "arp: Insert new entry into cache %h->%u.%u.%u.%u \n", mac, ip(0), ip(1), ip(2), ip(3));

    cache.emplace_back(mac, ip);
}

bool network::arp::is_mac_cached(uint64_t mac){
    for(auto& entry : cache){
        if(entry.mac == mac){
            return true;
        }
    }

    return false;
}

bool network::arp::is_ip_cached(network::ip::address ip){
    for(auto& entry : cache){
        if(entry.ip == ip){
            return true;
        }
    }

    return false;
}

network::ip::address network::arp::get_ip(uint64_t mac){
    thor_assert(is_mac_cached(mac), "The MAC is not cached in the ARP table");

    for(auto& entry : cache){
        if(entry.mac == mac){
            return entry.ip;
        }
    }

    thor_unreachable("The MAC is not cached in the ARP table");
}

uint64_t network::arp::get_mac(network::ip::address ip){
    thor_assert(is_ip_cached(ip), "The IP is not cached in the ARP table");

    for(auto& entry : cache){
        if(entry.ip == ip){
            return entry.mac;
        }
    }

    thor_unreachable("The IP is not cached in the ARP table");
}

uint64_t network::arp::get_mac_force(network::interface_descriptor& interface, network::ip::address ip){
    if(is_ip_cached(ip)){
        return get_mac(ip);
    }

    // At this point we need to send a request for the IP

    logging::logf(logging::log_level::TRACE, "arp: IP not cached, generate ARP Request\n");

    // Ask the ethernet layer to craft a packet
    auto packet = network::ethernet::prepare_packet(interface, sizeof(network::arp::header), 0xFFFFFFFFFFFF, ethernet::ether_type::ARP);

    auto* arp_request_header = reinterpret_cast<network::arp::header*>(packet.payload + packet.index);

    arp_request_header->hw_type = switch_endian_16(0x1); // ethernet
    arp_request_header->protocol_type = switch_endian_16(0x800); // IPV4
    arp_request_header->hw_len = 0x6; // MAC Address
    arp_request_header->protocol_len = 0x4; // IP Address
    arp_request_header->operation = switch_endian_16(0x1); //ARP Request

    auto source_mac = interface.mac_address;

    for(size_t i = 0; i < 3; ++i){
        arp_request_header->target_hw_addr[i] = 0x0;
        arp_request_header->source_hw_addr[i] = switch_endian_16(uint16_t(source_mac >> ((2 - i) * 16)));
    }

    network::ip::address source = network::ip::make_address(10,0,2,15);

    for(size_t i = 0; i < 2; ++i){
        arp_request_header->source_protocol_addr[i] = (uint16_t(source(2*i+1)) << 8) + source(2*i);
        arp_request_header->target_protocol_addr[i] = (uint16_t(ip(2*i+1)) << 8) + ip(2*i);
    }

    network::ethernet::finalize_packet(interface, packet);

    while(!is_ip_cached(ip)){
        network::arp::wait_for_reply();
    }

    logging::logf(logging::log_level::TRACE, "arp: received ARP Reply\n");

    return get_mac(ip);
}

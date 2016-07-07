//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector.hpp>
#include <string.hpp>

#include "arp_cache.hpp"
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

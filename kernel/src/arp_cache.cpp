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

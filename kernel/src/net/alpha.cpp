//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/alpha.hpp"
#include "net/ethernet_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/dns_layer.hpp"

#include "logging.hpp"
#include "kernel_utils.hpp"
#include "scheduler.hpp"

#include "tlib/errors.hpp"

void network::alpha(){
    auto& interface = network::interface(0);

    auto target_ip = network::ip::make_address(10,0,2,3);

    std::string domain("www.google.ch");

    auto parts = std::split(domain, '.');

    size_t characters = domain.size() - (parts.size() - 1); // The dots are not included
    size_t labels = parts.size();

    size_t payload_size = labels + characters + 1 + 2 * 2;

    // Ask the DNS layer to craft a packet
    network::dns::packet_descriptor desc{payload_size, target_ip, 3456, 0x666, true};
    auto packet = network::dns::kernel_prepare_packet_query(interface, desc);

    if(packet){
        auto* payload = reinterpret_cast<char*>(packet->payload + packet->index);

        size_t i = 0;
        for(auto& part : parts){
            payload[i++] = part.size();

            for(size_t j = 0; j < part.size(); ++j){
                payload[i++] = part[j];
            }
        }

        payload[i++] = 0;

        auto* q_type = reinterpret_cast<uint16_t*>(packet->payload + packet->index + i);
        *q_type = 0x0100; // A Record

        auto* q_class = reinterpret_cast<uint16_t*>(packet->payload + packet->index + i + 2);
        *q_class = 0x0100; // IN (internet)

        // Send the packet back to ICMP
        network::dns::finalize_packet(interface, *packet);
    }
}

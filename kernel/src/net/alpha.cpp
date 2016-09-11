//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/alpha.hpp"
#include "net/ethernet_layer.hpp"
#include "net/icmp_layer.hpp"
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

    size_t payload_size = 1 + domain.size() + 2 * 2;

    // Ask the DNS layer to craft a packet
    auto packet = network::dns::prepare_packet_query(interface, target_ip, 3456, 666, payload_size);

    if(packet){
        auto* payload = reinterpret_cast<char*>(packet->payload + packet->index);

        payload[0] = domain.size();

        for(size_t i = 0; i < domain.size(); ++i){
            payload[i + 1] = domain[i];
        }

        auto* q_type = reinterpret_cast<uint16_t*>(packet->payload + packet->index + 1 + domain.size());
        *q_type = 1; // A Record

        auto* q_class = q_type + 1;
        *q_class = 1; // IN (internet)

        // Send the packet back to ICMP
        network::icmp::finalize_packet(interface, *packet);
    }
}

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "net/dns_layer.hpp"

#include "kernel_utils.hpp"

namespace {


} //end of anonymous namespace

void network::dns::decode(network::interface_descriptor& /*interface*/, network::ethernet::packet& packet){
    packet.tag(3, packet.index);

    auto* dns_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "dns: Start DNS packet handling\n");

    auto identification = switch_endian_16(dns_header->identification);
    auto questions      = switch_endian_16(dns_header->questions);
    auto answers        = switch_endian_16(dns_header->answers);

    logging::logf(logging::log_level::TRACE, "dns: Identification %h \n", size_t(identification));
    logging::logf(logging::log_level::TRACE, "dns: Questions %h \n", size_t(questions));
    logging::logf(logging::log_level::TRACE, "dns: Answers %h \n", size_t(answers));

    auto flags = dns_header->flags;

    if(flags.qr){
        logging::logf(logging::log_level::TRACE, "dns: Query\n");
    } else {
        logging::logf(logging::log_level::TRACE, "dns: Response\n");
    }

    //TODO
}

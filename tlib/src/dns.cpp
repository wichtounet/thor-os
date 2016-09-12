//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/dns.hpp"
#include "tlib/malloc.hpp"

std::string tlib::dns::decode_domain(char* payload, size_t& offset) {
    std::string domain;

    offset = 0;

    while (true) {
        auto label_size = static_cast<uint8_t>(*(payload + offset));
        ++offset;

        if (!label_size) {
            break;
        }

        if (!domain.empty()) {
            domain += '.';
        }

        for (size_t i = 0; i < label_size; ++i) {
            domain += *(payload + offset);
            ++offset;
        }
    }

    return domain;
}

std::expected<void> tlib::dns::send_request(tlib::socket& sock, const std::string& domain, uint16_t rr_type, uint16_t rr_class){
    auto parts = std::split(domain, '.');

    size_t characters = domain.size() - (parts.size() - 1); // The dots are not included
    size_t labels     = parts.size();

    tlib::dns::packet_descriptor desc;
    desc.payload_size   = labels + characters + 1 + 2 * 2;
    desc.target_ip      = tlib::ip::make_address(10, 0, 2, 3);
    desc.source_port    = 3456;
    desc.identification = 0x666;
    desc.query          = true;

    auto packet = sock.prepare_packet(&desc);

    if (!sock) {
        return std::make_unexpected<void>(sock.error());
    }

    auto* payload = reinterpret_cast<char*>(packet.payload + packet.index);

    size_t i = 0;
    for (auto& part : parts) {
        payload[i++] = part.size();

        for (size_t j = 0; j < part.size(); ++j) {
            payload[i++] = part[j];
        }
    }

    payload[i++] = 0;

    auto* q_type = reinterpret_cast<uint16_t*>(packet.payload + packet.index + i);
    *q_type      = tlib::switch_endian_16(rr_type);

    auto* q_class = reinterpret_cast<uint16_t*>(packet.payload + packet.index + i + 2);
    *q_class      = tlib::switch_endian_16(rr_class);

    sock.finalize_packet(packet);

    if (!sock) {
        return std::make_unexpected<void>(sock.error());
    }

    return {};
}

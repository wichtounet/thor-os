//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/dns.hpp"
#include "tlib/malloc.hpp"
#include "tlib/system.hpp"
#include "tlib/errors.hpp"
#include "tlib/print.hpp"

namespace {

bool is_digit(char c){
    return c >= '0' && c <= '9';
}

} // end of anonymous namespace

bool tlib::dns::is_ip(const std::string& value){
    auto ip_parts = std::split(value, '.');

    if(ip_parts.size() != 4){
        return false;
    }

    for(auto& part : ip_parts){
        if(part.empty() || part.size() > 3){
            return false;
        }

        for(size_t i = 0; i < part.size(); ++i){
            if(!is_digit(part[i])){
                return false;
            }
        }
    }

    return true;
}

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

tlib::ip::address tlib::dns::gateway_address(){
    uint64_t ret;
    asm volatile("mov rax, 0xB15; int 50; mov %[code], rax"
                 : [code] "=m"(ret)
                 :
                 : "rax");

    return {uint32_t(ret)};
}

std::expected<void> tlib::dns::send_request(tlib::socket& sock, const std::string& domain, uint16_t rr_type, uint16_t rr_class){
    auto parts = std::split(domain, '.');

    size_t characters = domain.size() - (parts.size() - 1); // The dots are not included
    size_t labels     = parts.size();

    tlib::dns::packet_descriptor desc;
    desc.payload_size   = labels + characters + 1 + 2 * 2;
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

std::expected<tlib::ip::address> tlib::dns::resolve(const std::string& domain, size_t timeout_ms, size_t retries){
    tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::DGRAM, tlib::socket_protocol::DNS);

    sock.client_bind(gateway_address());
    sock.listen(true);

    if (!sock) {
        return std::make_unexpected<tlib::ip::address>(sock.error());
    }

    size_t tries = 0;

    auto sr = send_request(sock, domain, 0x1, 0x1);
    if(!sr){
        return std::make_unexpected<tlib::ip::address>(sr.error());
    }

    auto before = tlib::ms_time();
    auto after  = before;

    while (true) {
        // Make sure we don't wait for more than the timeout
        if (after > before + timeout_ms) {
            break;
        }

        auto remaining = timeout_ms - (after - before);

        auto p = sock.wait_for_packet(remaining);
        if (!sock) {
            return std::make_unexpected<tlib::ip::address>(sock.error());
        } else {
            auto* dns_header = reinterpret_cast<tlib::dns::header*>(p.payload + p.index);

            auto identification = tlib::switch_endian_16(dns_header->identification);

            // Only handle packet with the correct identification
            if (identification == 0x666) {
                auto questions = tlib::switch_endian_16(dns_header->questions);
                auto answers   = tlib::switch_endian_16(dns_header->answers);

                auto flags = tlib::switch_endian_16(dns_header->flags);
                auto qr    = flags >> 15;

                // Only handle Response
                if (qr) {
                    auto rcode = flags & 0xF;

                    if (rcode == 0x0 && answers > 0) {
                        auto* payload = p.payload + p.index + sizeof(tlib::dns::header);

                        // Decode the questions (simply wrap around it)

                        for (size_t i = 0; i < questions; ++i) {
                            size_t length;
                            tlib::dns::decode_domain(payload, length);

                            payload += length;
                            payload += 4;
                        }

                        for (size_t i = 0; i < answers; ++i) {
                            auto label = static_cast<uint8_t>(*payload);

                            if (label > 64) {
                                // This is a pointer
                                auto pointer = tlib::switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                                auto offset  = pointer & (0xFFFF >> 2);

                                payload += 2;

                                size_t ignored;
                                tlib::dns::decode_domain(p.payload + p.index + offset, ignored);
                            } else {
                                return std::make_unexpected<tlib::ip::address>(std::ERROR_UNSUPPORTED);
                            }

                            auto rr_type = tlib::switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                            payload += 2;

                            auto rr_class = tlib::switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                            payload += 2;

                            payload += 4; // TTL

                            auto rd_length = tlib::switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                            payload += 2;

                            if(rr_class == 1){ // IN (Internet) class
                                if (rr_type == 0x1) { // A record
                                    auto ip = reinterpret_cast<uint8_t*>(payload);

                                    auto result = tlib::ip::make_address(ip[3], ip[2], ip[1], ip[0]);
                                    return std::make_expected<tlib::ip::address>(result);
                                }
                            }

                            payload += rd_length;
                        }

                        break;
                    } else {
                        // There was an error, retry
                        if(++tries == retries || !send_request(sock, domain)){
                            return std::make_unexpected<tlib::ip::address>(std::ERROR_SOCKET_TIMEOUT);
                        }
                    }
                }
            }
        }

        after = tlib::ms_time();
    }

    return std::make_unexpected<tlib::ip::address>(std::ERROR_SOCKET_TIMEOUT);
}

std::expected<std::string> tlib::dns::resolve_str(const std::string& domain, size_t timeout_ms, size_t retries){
    auto result = resolve(domain, timeout_ms, retries);

    if(result){
        auto& ip = *result;
        auto result = sprintf("%u.%u.%u.%u", ip(3), ip(2), ip(1), ip(0));
        return std::make_expected<std::string>(result);
    }

    return std::make_unexpected<std::string>(result.error());
}

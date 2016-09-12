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

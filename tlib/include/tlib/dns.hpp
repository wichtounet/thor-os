//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TLIB_NET_DNS_H
#define TLIB_NET_DNS_H

#include <string.hpp>

#include "tlib/net_constants.hpp"
#include "tlib/net.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

namespace dns {

std::string decode_domain(char* payload, size_t& offset);
std::expected<void> send_request(tlib::socket& sock, const std::string& domain, uint16_t rr_type = 0x1, uint16_t rr_class = 0x1);

std::expected<tlib::ip::address> resolve(const std::string& domain, size_t timeout = 1000, size_t retries = 1);
std::expected<std::string> resolve_str(const std::string& domain, size_t timeout = 1000, size_t retries = 1);

tlib::ip::address gateway_address();

bool is_ip(const std::string& value);

}  // end of namespace dns

} // end of namespace tlib

#endif

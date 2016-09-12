//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TLIB_NET_DNS_H
#define TLIB_NET_DNS_H

#include <string.hpp>

#include "tlib/net_constants.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

namespace dns {

std::string decode_domain(char* payload, size_t& offset);

}  // end of namespace dns

} // end of namespace tlib

#endif

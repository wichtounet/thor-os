//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GDT_H
#define GDT_H

namespace gdt {

constexpr const uint16_t CODE_SELECTOR = 0x08;
constexpr const uint16_t DATA_SELECTOR = 0x10;
constexpr const uint16_t LONG_SELECTOR = 0x18;

} //end of namespace gdt

#endif

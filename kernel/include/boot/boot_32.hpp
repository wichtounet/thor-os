//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOT_32_HPP
#define BOOT_32_HPP

void __attribute((section("boot_32_section"), noreturn)) pm_main();

#endif

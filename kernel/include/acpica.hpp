//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef THOR_ACPICA_HPP
#define THOR_ACPICA_HPP

extern "C" {

#include "thor_acenv.hpp"
#include "thor_acenvex.hpp"

//ACPICA
#include <acpi.h>
#include <accommon.h>

} //end of extern "C"

#include <types.hpp>

constexpr const size_t FADT2_REVISION_ID = 3;

#endif

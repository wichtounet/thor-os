//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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

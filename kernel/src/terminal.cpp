//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "stl/vector.hpp"
#include "terminal.hpp"

namespace {

constexpr const size_t DEFAULT_TERMINALS = 1;
size_t active_terminal;

std::vector<stdio::virtual_terminal> terminals;

} //end of anonymous namespace

void stdio::init_terminals(){
    terminals.resize(DEFAULT_TERMINALS);

    for(size_t i  = 0; i < DEFAULT_TERMINALS; ++i){
        terminals[i].id = i;
        terminals[i].active = false;
    }

    active_terminal = 0;
    terminals[active_terminal].active = true;
}

stdio::virtual_terminal& stdio::get_active_terminal(){
    return terminals[active_terminal];
}

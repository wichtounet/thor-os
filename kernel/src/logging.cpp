//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"
#include "assert.hpp"

namespace {

bool early = true;
bool file = false;

constexpr const size_t MAX_EARLY = 128;

size_t current_early = 0;
const char* early_logs[MAX_EARLY];

} //end of anonymous namespace

bool logging::is_early(){
    return early;
}

bool logging::is_file(){
    return file;
}

void logging::finalize(){
    //Starting from there, the messages will be displayed on
    //the screen
    early = false;
}

void logging::to_file(){
    file = true;

    //TODO
}

void logging::log(const char* s){
    if(is_early()){
        thor_assert(current_early < MAX_EARLY, "early log buffer is full");

        early_logs[current_early++] = s;
    } else {
        if(is_file()){
            //TODO
        } else {
            //TODO
        }
    }
}

void logging::log(const std::string& s){
    thor_assert(!is_early(), "log(string) in only valid in normal mode");

    if(is_file()){
        //TODO
    } else {
        //TODO
    }
}

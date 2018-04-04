//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/print.hpp>

int main(int argc, char* argv[]){
    for(size_t i = 0; i < size_t(argc); ++i){
        tlib::printf("arg:%u:%p:%s\n", i, argv[i], argv[i]);
    }

    return 0;
}

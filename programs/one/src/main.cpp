//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

int main(){
    asm volatile("mov rax, 0; mov rbx, 0x41; int 50" : : : "rax", "rbx");

    return 1;
}
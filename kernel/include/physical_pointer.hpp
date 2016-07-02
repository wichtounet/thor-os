//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PHYSICAL_POINTER_H
#define PHYSICAL_POINTER_H

#include "virtual_allocator.hpp"
#include "paging.hpp"

struct physical_pointer {
private:
    size_t phys;
    size_t pages;
    size_t virt;

public:
    physical_pointer(size_t phys_p, size_t pages_p) : phys(phys_p), pages(pages_p) {
        if(pages > 0){
            virt = virtual_allocator::allocate(pages);

            if(virt){
                if(pages == 1){
                    if(!paging::map(virt, phys)){
                        virt = 0;
                    }
                } else {
                    if(!paging::map_pages(virt, phys, pages)){
                        virt = 0;
                    }
                }
            }
        } else {
            virt = 0;
        }
    }

    ~physical_pointer(){
        if(virt){
            if(pages == 1){
                paging::unmap(virt);
            } else {
                paging::unmap_pages(virt, pages);
            }

            virtual_allocator::free(virt, pages);
        }
    }

    template<typename T>
    T as(){
        return reinterpret_cast<T>(virt);
    }

    template<typename T>
    T* as_ptr(){
        return reinterpret_cast<T*>(virt);
    }

    uintptr_t get(){
        return virt;
    }

    operator bool() const {
        return virt != 0;
    }
};

#endif

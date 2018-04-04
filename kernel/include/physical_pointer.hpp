//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef PHYSICAL_POINTER_H
#define PHYSICAL_POINTER_H

#include <type_traits.hpp>

#include "virtual_allocator.hpp"
#include "paging.hpp"

/*!
 * \brief A special pointer to physical memory
 */
struct physical_pointer {
    /*!
     * \brief Creates a new physical pointer
     * \param phys_p Physical address
     * \param pages_p The nubmer of pages
     */
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

    /*!
     * \brief Destroys the physical pointer and releases its memory
     */
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

    /*!
     * \brief Reinterprets the virtual memory as the given type.
     *
     * This should
     *
     */
    template<typename T>
    T as(){
        static_assert(std::is_pointer<T>::value, "as<T>() should only be used with pointer types");
        return reinterpret_cast<T>(virt);
    }

    /*!
     * \brief Returns a pointer of a given type to the memory
     */
    template<typename T>
    T* as_ptr(){
        return reinterpret_cast<T*>(virt);
    }

    /*!
     * \brief Returns the virtual address
     */
    uintptr_t get() const {
        return virt;
    }

    /*!
     * \brief Returns the physical address
     */
    uintptr_t get_phys() const {
        return virt;
    }

    /*!
     * \brief Convert the pointer to a boolean
     */
    operator bool() const {
        return virt != 0;
    }

private:
    const size_t phys; ///< The physical memory address
    const size_t pages; ///< The number of pages
    size_t virt; ///< The virtual memory

};

#endif

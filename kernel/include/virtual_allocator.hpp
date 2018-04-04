//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef VIRTUAL_ALLOCATOR_H
#define VIRTUAL_ALLOCATOR_H

#include <types.hpp>
#include <literals.hpp>

namespace virtual_allocator {

//The virtual size allocated to the kernel
constexpr const size_t kernel_virtual_size = 1_GiB;

/*
 * \brief Initialize the vitual allocator.
 *
 * After this point, the early_allocate function should not be called
 */
void init();

/*!
 * \brief Finalize the vitual allocator, must be called after sysfs initialization
 */
void finalize();

/*!
 * \brief Allocate several pages of vitual memory
 * \param pages The number of pages
 * \return The vitual addres of the allocated pages
 */
size_t allocate(size_t pages);

/*!
 * \brief Free the allocated virtual memory
 * \param address The address of the allocated virtual memory
 * \param pages The number of pages
 */
void free(size_t address, size_t pages);

/*!
 * \brief Return the amount of virtual memory available
 */
size_t available();

/*!
 * \brief Return the amount of virtual memory allocated
 */
size_t allocated();

/*!
 * \brief Return the amount of virtual memory free
 */
size_t free();

} //end of virtual_allocator namespace

#endif

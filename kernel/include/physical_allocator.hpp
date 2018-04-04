//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef PHYSICAL_ALLOCATOR_H
#define PHYSICAL_ALLOCATOR_H

#include <types.hpp>

namespace physical_allocator {

/*!
 * \brief Early initialization of the physical allocator.
 *
 * This finalizes the e820 memory detection and start preparing memory from one of the blocks
 */
void early_init();

/*!
 * \brief Early allocation of physical memory pages
 * \param pages The number of pages to allocate
 */
size_t early_allocate(size_t pages);

/*
 * \brief Initialize the real physical allocator.
 *
 * After this point, the early_allocate function should not be called
 */
void init();

/*!
 * \brief Finalize the physical allocator, must be called after sysfs initialization
 */
void finalize();

/*!
 * \brief Allocate several pages of physical memory
 * \param pages The number of pages
 * \return The physical addres of the allocated pages
 */
size_t allocate(size_t pages);

/*!
 * \brief Free the allocated physical memory
 * \param address The address of the allocated physical memory
 * \param pages The number of pages
 */
void free(size_t address, size_t pages);

/*!
 * \brief Return the amount of physical memory available
 */
size_t total_available();

/*!
 * \brief Return the amount of physical memory currently available
 */
size_t available();

/*!
 * \brief Return the amount of physical memory allocated
 */
size_t total_allocated();

/*!
 * \brief Return the amount of physical memory currently allocated
 */
size_t allocated();

/*!
 * \brief Return the amount of physical memory currently free
 */
size_t free();

/*!
 * \brief Return the amount of physical memory free
 */
size_t total_free();

} //end of physical_allocator namespace

#endif

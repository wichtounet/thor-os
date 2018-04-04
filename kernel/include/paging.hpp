//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef PAGING_H
#define PAGING_H

#include <types.hpp>
#include <literals.hpp>

#include "virtual_allocator.hpp"

//Forward declaration
namespace scheduler {
struct process_t;
}

namespace paging {

constexpr const size_t PAGE_SIZE = 4096; ///< The size of a page (4K)

constexpr const size_t pml4e_allocations = 512_GiB; ///< The physical memory that a PML4T Entry can map
constexpr const size_t pdpte_allocations = 1_GiB;   ///< The physical memory that a PDPT Entry can map
constexpr const size_t pde_allocations   = 2_MiB;   ///< The physical memory that a PD Entry can map
constexpr const size_t pte_allocations   = 4_KiB;   ///< The physical memory that a PD Entry can map

//Help function to compute the number of necessary entries to map the kernel
constexpr size_t entries(size_t entry_size){
    return entry_size >= virtual_allocator::kernel_virtual_size ?
        1 :
        virtual_allocator::kernel_virtual_size / entry_size + (virtual_allocator::kernel_virtual_size % entry_size == 0 ? 0 : 1);
}

//Compute the number of entries necessary to map the entire kernel virtual size

constexpr const auto pml4_entries = entries(pml4e_allocations); ///< Number of PML4T entries necessary to map the kernel
constexpr const auto pdpt_entries = entries(pdpte_allocations); ///< Number of PDPT entries necessary to map the kernel
constexpr const auto pd_entries   = entries(pde_allocations);   ///< Number of PD entries necessary to map the kernel
constexpr const auto pt_entries   = entries(pte_allocations);   ///< Number of PT entries necessary to map the kernel

//Compute the amount of physical memory pages needed for the paging tables
constexpr const size_t  physical_memory_pages = 1 + pml4_entries + pdpt_entries + pd_entries;

//Virtual address where early page can be mapped
extern size_t virtual_early_page;

//Virtual address where the paging structures are stored
extern size_t virtual_paging_start;

//Compute the start address of each structure
extern size_t virtual_pml4t_start;
extern size_t virtual_pdpt_start;
extern size_t virtual_pd_start;
extern size_t virtual_pt_start;

constexpr const uint8_t PRESENT        = 0x1;  ///< Paging flag for present page
constexpr const uint8_t WRITE          = 0x2;  ///< Paging flag for writable page
constexpr const uint8_t USER           = 0x4;  ///< Paging flag for user page
constexpr const uint8_t WRITE_THROUGH  = 0x8;  ///< Paging flag for write-through page
constexpr const uint8_t CACHE_DISABLED = 0x10; ///< Paging flag for cache disabled page
constexpr const uint8_t ACCESSED       = 0x20; ///< Paging flag for assessed page

/*!
 * \brief Test if an address is aligned on a page boundary
 */
constexpr bool page_aligned(size_t addr){
    return !(addr & (paging::PAGE_SIZE - 1));
}

constexpr size_t page_align(size_t addr){
    return (addr / paging::PAGE_SIZE) * paging::PAGE_SIZE;
}

/*!
 * \brief Early initialization of the paging manager. This is done
 * before the virtual and physical allocators are initialized.
 */
void early_init();

/*!
 * \brief Initialize the paging manager (after phyiscal and virtual
 * allocators are initialized.
 */
void init();

/*!
 * \brief Finalize the initialization, after sysfs initialization
 */
void finalize();

/*!
 * \brief Returns the number of pages for a block of the given size
 */
size_t pages(size_t size);

/*!
 * \brief Returns the physical address pointed by the given virtual address
 * \param virt The virtual address
 * \return The corresponding physical address if any, 0 otherwise
 */
size_t physical_address(size_t virt);

/*!
 * \brief Indicates if the virtual page is present or not
 */
bool page_present(size_t virt);

/*!
 * \brief Indicates if the virtual page is free or already set to
 * the given physical address
 * \param virt The virtual address
 */
bool page_free_or_set(size_t virt, size_t physical);

/*!
 * \brief Identity map the given virtual page
 * \param flag The flags to set
 * \return true if paging is possible, false otherwise
 */
bool identity_map(size_t virt, uint8_t flags = PRESENT | WRITE);

/*!
 * \brief Identity map the given virtual pages
 * \param virt The first virtual page
 * \param pages The number of pages to identity map
 * \param flag The flags to set
 * \return true if paging is possible, false otherwise
 */
bool identity_map_pages(size_t virt, size_t pages, uint8_t flags = PRESENT | WRITE);

/*!
 * \brief Map the given virtual page to the given physical page
 * \param virt The virtual page
 * \param physical The physical page
 * \param flag The flags to set
 * \return true if paging is possible, false otherwise
 */
bool map(size_t virt, size_t physical, uint8_t flags = PRESENT | WRITE);

/*!
 * \brief Map the given virtual pages to the given physical pages
 * \param virt The first virtual page
 * \param physical The first physical page
 * \param pages The number of pages to map
 * \param flag The flags to set
 * \return true if paging is possible, false otherwise
 */
bool map_pages(size_t virt, size_t physical, size_t pages, uint8_t flags = PRESENT | WRITE);

/*!
 * \brief Unmap the virtual page
 * \return true if unmap is possible, false otherwise
 */
bool unmap(size_t virt);

/*!
 * \brief Unmap the virtual pages
 * \þaram virt The first virtual page
 * \þaram pages The number of pages to unmap
 * \return true if unmap is possible, false otherwise
 */
bool unmap_pages(size_t virt, size_t pages);

/*!
 * \brief Map the entire kernel inside the user process
 * \param process The proces to manipulate
 */
void map_kernel_inside_user(scheduler::process_t& process);

/*!
 * \brief Map the given virtual page to the given physical page for the given process
 * \param virt The virtual page
 * \param physical The physical page
 * \return true if paging is possible, false otherwise
 */
bool user_map(scheduler::process_t& process, size_t virt, size_t physical);

/*!
 * \brief Map the given virtual pages to the given physical page for the given process
 * \param virt The first virtual page
 * \param physical The first physical page
 * \þaram pages The number of pages to map
 * \return true if paging is possible, false otherwise
 */
bool user_map_pages(scheduler::process_t& process, size_t virt, size_t physical, size_t pages);

/*!
 * \brief Returns the physical address of the PML4T table
 */
size_t get_physical_pml4t();

} //end of namespace paging

#endif

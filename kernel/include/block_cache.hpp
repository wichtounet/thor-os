//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef BLOCK_CACHE_HPP
#define BLOCK_CACHE_HPP

#include <types.hpp>

/*!
 * \brief A block in the block cache
 */
struct block_t {
    uint64_t key; ///< The key of the block
    block_t* hash_next; ///< The next block in the hash map bucket
    block_t* free_next; ///< The next block in the free list
    block_t* free_prev; ///< The previous block in the free list
    char payload; ///< The start of the payload
} __attribute__((packed));

/*!
 * \brief A cache for I/O blocks
 */
struct block_cache {
    /*!
     * \brief Initialize the cache
     * \param payload_size The size of each block
     * \param blocks The number of blocks to cache
     */
    void init(uint64_t payload_size, uint64_t blocks);

    /*!
     * \brief Returns the block at the given position if it exists
     * \return the block payload address if there is a block,nullptr otherwise
     */
    char* block_if_present(uint16_t device, uint64_t sector);

    /*!
     * \brief Returns the block at the given position if it exists
     * \return the block payload address if there is a block,nullptr otherwise
     */
    char* block_if_present(uint64_t key);

    /*!
     * \brief Returns the block at the given position if it exists
     * \param valid An output parameter indicating if the block is valid or new (false)
     * \return the block payload address
     */
    char* block(uint16_t device, uint64_t sector, bool& valid);

    /*!
     * \brief Returns the block at the given position if it exists
     * \param valid An output parameter indicating if the block is valid or new (false)
     * \return the block payload address
     */
    char* block(uint64_t key, bool& valid);

private:
    uint64_t payload_size; ///< The size of each blocks
    uint64_t blocks; ///< The number of blocks to cache

    void* blocks_memory; ///< The memory holding the blocks

    block_t** hash_table; ///< Pointer to the hash table

    block_t* front; ///< The head of the free list
    block_t* rear;  ///< The tail of the free list
};

#endif

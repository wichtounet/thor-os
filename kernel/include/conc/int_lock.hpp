//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef INT_LOCK_HPP
#define INT_LOCK_HPP

#include <types.hpp>

#include "arch.hpp"

/*!
 * \brief An interrupt lock. This lock disable preemption on acquire.
 */
struct int_lock {
    /*!
     * \brief Acquire the lock. This will disable preemption.
     */
    void lock() {
        arch::disable_hwint(rflags);
    }

    /*!
     * \brief Release the lock. This will enable preemption.
     */
    void unlock() {
        arch::enable_hwint(rflags);
    }

private:
    size_t rflags; ///< The CPU flags
};

/*!
 * \brief A direct interrupt lock (RAII).
 *
 * This is the equivalent of a std::lock_guard<int_lock> but does not need to
 * store a lock.
 */
struct direct_int_lock {
    /*!
     * \brief Construct a new direct_int_lock and acquire the lock.
     */
    direct_int_lock() {
        lock.lock();
    }

    /*!
     * \brief Destruct a direct_int_lock and release the lock.
     */
    ~direct_int_lock() {
        lock.unlock();
    }

private:
    int_lock lock; ///< The interrupt lock
};

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef SPINLOCK_H
#define SPINLOCK_H

/*!
 * \brief Implementation of a spinlock
 *
 * A spinlock simply waits in a loop until the lock is available.
 */
struct spinlock {
    /*!
     * \brief Acquire the lock.
     *
     * This will wait indefinitely.
     */
    void lock(){
        while(!__sync_bool_compare_and_swap(&value, 0, 1));
        __sync_synchronize();
        //TODO The last synchronize is probably not necessary
    }

    /*!
     * \brief Release the lock
     */
    void unlock(){
        __sync_synchronize();
        value = 0;
    }

private:
    volatile size_t value = 0; ///< The value of the lock
};

#endif

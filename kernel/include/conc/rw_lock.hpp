//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef RW_LOCK_H
#define RW_LOCK_H

#include "conc/mutex.hpp"
#include "conc/condition_variable.hpp"

struct rw_lock;

/*!
 * \brief A reader lock from a RW lock.
 *
 * This should be used to make code clearer and to use std::lock_guard
 */
struct reader_rw_lock final {
    /*!
     * \brief Construct a new reader_rw_lock from a rw_lock
     */
    reader_rw_lock(rw_lock& l) : l(l) {
        //Nothing else to init
    }

    /*!
     * \brief Acquire the reader lock
     */
    void lock();

    /*!
     * \brief Release the reader lock
     */
    void unlock();

private:
    rw_lock& l; ///< The parent rw_lock
};

/*!
 * \brief A writer lock from a RW lock.
 *
 * This should be used to make code clearer and to use std::lock_guard
 */
struct writer_rw_lock final {
    /*!
     * \brief Construct a new writer_rw_lock from a rw_lock
     */
    writer_rw_lock(rw_lock& l) : l(l) {
        //Nothing else to init
    }

    /*!
     * \brief Acquire the writer lock
     */
    void lock();

    /*!
     * \brief Release the writer lock
     */
    void unlock();

private:
    rw_lock& l; ///< The parent rw_lock
};

/*!
 * \brief A Read/Write lock implementation.
 *
 * There can be multiple readers, but only one writer. The writer has exclusive
 * access.
 */
struct rw_lock final {
    /*!
     * \brief Acquire the lock for reading
     */
    void read_lock(){
        m.lock();

        while(writer){
            m.unlock();
            write.wait();
            m.lock();
        }

        ++readers;

        m.unlock();
    }

    /*!
     * \brief Release the lock for reading
     */
    void read_unlock(){
        m.lock();

        --readers;

        // If there are no more readers, notify one writer
        if(!readers){
            write.notify_one();
        }

        m.unlock();
    }

    /*!
     * \brief Acquire the lock for writing.
     */
    void write_lock(){
        m.lock();

        while(writer || readers){
            m.unlock();
            write.wait();
            m.lock();
        }

        writer = true;

        m.unlock();
    }

    /*!
     * \brief Release the lock for writing.
     */
    void write_unlock(){
        m.lock();

        writer = false;

        // Notify all writers and readers
        write.notify_all();

        m.unlock();
    }

    /*!
     * \brief Returns a lock for reader
     */
    reader_rw_lock reader_lock(){
        return {*this};
    }

    /*!
     * \brief Returns a lock for writer
     */
    writer_rw_lock writer_lock(){
        return {*this};
    }

private:
    condition_variable write; ///< The write condition variable
    mutex m;                  ///< Mutex protecting the counter
    size_t readers = 0;       ///< Number of readers
    bool writer    = false;   ///< Boolean flag indicating if there is a writer
};

inline void writer_rw_lock::lock(){
    l.read_lock();
}

inline void writer_rw_lock::unlock(){
    l.read_unlock();
}

inline void reader_rw_lock::lock(){
    l.write_lock();
}

inline void reader_rw_lock::unlock(){
    l.write_unlock();
}

#endif

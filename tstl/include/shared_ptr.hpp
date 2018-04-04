//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef SHARED_PTR_H
#define SHARED_PTR_H

#include <tuple.hpp>
#include <algorithms.hpp>
#include <deleter.hpp>

namespace std {

/*!
 * \brief A shared_ptr ptr of type T.
 *
 * A shared pointer is a reference-counted pointer, with several pointers being
 * able to point to the same managed object.  When the reference counter goes
 * down to zero, the object is automatically deleted.
 */
template <typename T>
struct shared_ptr {
    using pointer_type = T*; ///< The pointer type
    using reference_type = T&; ///< The reference type
    using element_type = T;  ///< The element type

    struct control_block_t;

    /*!
     * \brief Construct an empty shared_ptr
     */
    constexpr shared_ptr() : ptr(nullptr), control_block(nullptr) {}

    /*!
     * \brief Construct an empty shared_ptr
     */
    constexpr explicit shared_ptr(decltype(nullptr)) : ptr(nullptr), control_block(nullptr) {}

    /*!
     * \brief Construct a new shared_ptr around the given pointer.
     *
     * The pointer will be deleted using the 'delete-expression'
     */
    template<typename U>
    explicit shared_ptr(U* ptr) : ptr(ptr) {
        control_block = new control_block_impl<U, default_delete<U>>(ptr);
    }

    /*!
     * \brief Construct a new shared_ptr around the given pointer.
     *
     * The pointer will be deleted using 'deleter(ptr)'
     */
    template<typename U, typename Deleter>
    shared_ptr(U* ptr, Deleter deleter) : ptr(ptr) {
        control_block = new control_block_impl<U, Deleter>(ptr, deleter);
    }

    /*!
     * \brief Construct a new shared_ptr directly with a control block
     */
    shared_ptr(T* ptr, control_block_t* control_block, int) : ptr(ptr), control_block(control_block) {
        //Nothing else to init
    }

    /*!
     * \brief Copy construct a shared_ptr, effectively incrementing the reference counter
     */
    shared_ptr(const shared_ptr& rhs) : ptr(rhs.ptr), control_block(rhs.control_block) {
        increment();
    }

    /*!
     * \brief Copy assign a shared_ptr, effectively incrementing the reference counter
     */
    shared_ptr& operator=(const shared_ptr& rhs){
        if(this != &rhs){
            decrement();

            this->ptr = rhs.ptr;
            this->control_block = rhs.control_block;

            increment();
        }

        return *this;
    }

    /*!
     * \brief Move construct a shared_ptr.
     *
     * This does not change the reference counter since the shared pointer moved from does not point to the object anymore
     */
    shared_ptr(shared_ptr&& rhs) : ptr(rhs.ptr), control_block(rhs.control_block) {
        rhs.ptr = nullptr;
        rhs.control_block = nullptr;
    }

    /*!
     * \brief Move assign a shared_ptr
     *
     * This does not change the reference counter since the shared pointer moved from does not point to the object anymore
     */
    shared_ptr& operator=(shared_ptr&& rhs){
        if(this != &rhs){
            decrement();

            this->ptr = rhs.ptr;
            this->control_block = rhs.control_block;

            rhs.ptr = nullptr;
            rhs.control_block = nullptr;
        }

        return *this;
    }

    /*!
     * \brief Assign a new pointer to the shared pointer.
     *
     * The pointer will be deleted using the 'delete-expression'
     */
    template<typename U>
    shared_ptr& operator=(U* ptr){
        decrement();

        this->ptr = ptr;

        control_block = new control_block_impl<U, default_delete<U>>(ptr);
    }

    /*!
     * \brief Resets the shared_ptr value
     */
    shared_ptr& operator=(decltype(nullptr)) {
        decrement();

        this->ptr = nullptr;
        this->control_block = nullptr;
    }

    /*!
     * \brief Destroy the shared_ptr. This effectively decrement the counter. If the counter reaches 0,
     * the object is deleted.
     */
    ~shared_ptr(){
        decrement();
    }

    /*!
     * \brief Returns the managed pointer
     */
    pointer_type get() const {
        return ptr;
    }

    /*!
     * \brief Returns the managed pointer
     */
    pointer_type operator->() const {
        return get();
    }

    /*!
     * \brief Returns a reference to the managed object
     */
    reference_type operator*() const {
        return *get();
    }

    /*!
     * \brief Converts the shared ptr to a boolean, indicating if it points to something or not
     */
    explicit operator bool() const {
        return get();
    }

    struct control_block_t {
        volatile size_t counter;

        control_block_t() : counter(1) {}

        control_block_t(const control_block_t& rhs) = delete;
        control_block_t& operator=(const control_block_t& rhs) = delete;

        control_block_t(control_block_t&& rhs) = delete;
        control_block_t& operator=(control_block_t&& rhs) = delete;

        virtual void destroy() = 0;
        virtual ~control_block_t(){}
    };

    template <typename U, typename Deleter>
    struct control_block_impl : control_block_t {
        U* ptr;
        Deleter deleter;

        control_block_impl(U* ptr) : ptr(ptr) {}
        control_block_impl(U* ptr, Deleter d) : ptr(ptr), deleter(d){}

        virtual void destroy() override {
            if(ptr){
                deleter(ptr);
            }
        }
    };

    template <typename U>
    struct inplace_control_block_impl : control_block_t {
        U ptr;

        template<typename... Args>
        inplace_control_block_impl(Args&&... args) : ptr(std::forward<Args>(args)...){}

        virtual void destroy() override {
            // Nothing to do
        }
    };

private:
    /*!
     * \brief Helper function to decrement the reference counter if the pointer points to a
     * managed object.
     *
     * If the counter goes to zero, this also deallocates the managed object and the control block
     */
    void decrement(){
        if(control_block){
            if(!__sync_sub_and_fetch(&control_block->counter, 1)){
                control_block->destroy();
                delete control_block;
            }
        }
    }

    /*!
     * \brief Helper function to increment the reference counter if the pointer points to a
     * managed object.
     */
    void increment(){
        if(control_block){
            __sync_fetch_and_add(&control_block->counter, 1);
        }
    }

    pointer_type ptr;               ///< The managed pointer
    control_block_t* control_block; ///< Pointer to the control block
};

/*!
 * \brief Creates a new shared_ptr.
 *
 * This is effectively more efficient than shared_ptr<T>(new T) since it allocates the object directly in the control
 * block, saving one allocation.
 */
template <typename T, typename... Args>
std::shared_ptr<T> make_shared(Args&&... args){
    auto cb = new typename std::shared_ptr<T>::template inplace_control_block_impl<T>(std::forward<Args>(args)...);

    return std::shared_ptr<T>{&cb->ptr, cb, 0};
}

} //end of namespace std

#endif

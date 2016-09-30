//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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
 * TODO
 */
template <typename T>
struct shared_ptr {
    using pointer_type = T*; ///< The pointer type
    using reference_type = T&; ///< The reference type
    using element_type = T;  ///< The element type

    struct control_block_t;

    constexpr shared_ptr() : ptr(), control_block() {}

    constexpr explicit shared_ptr(decltype(nullptr)) : ptr(), control_block() {}

    template<typename U>
    explicit shared_ptr(U* ptr) : ptr(ptr) {
        control_block = new control_block_impl<U, default_delete<U>>(ptr);
        control_block->counter = 1;
    }

    template<typename U, typename Deleter>
    shared_ptr(U* ptr, Deleter deleter) : ptr(ptr) {
        control_block = new control_block_impl<U, Deleter>(ptr, deleter);
        control_block->counter = 1;
    }

    shared_ptr(T* ptr, control_block_t* control_block, int) : ptr(ptr), control_block(control_block) {
        control_block->counter = 1;
    }

    shared_ptr(const shared_ptr& rhs) : ptr(rhs.ptr), control_block(rhs.control_block) {
        __sync_fetch_and_add(&control_block->counter, 1);
    }

    shared_ptr& operator=(const shared_ptr& rhs){
        if(this != &rhs){
            this->ptr = rhs.ptr;
            this->control_block = rhs.control_block;

            __sync_fetch_and_add(&control_block->counter, 1);
        }

        return *this;
    }

    shared_ptr(shared_ptr&& rhs) : ptr(rhs.ptr), control_block(rhs.control_block) {
        rhs.ptr = nullptr;
        rhs.control_block = nullptr;
    }

    shared_ptr& operator=(shared_ptr&& rhs){
        if(this != &rhs){
            this->ptr = rhs.ptr;
            this->control_block = rhs.control_block;

            rhs.ptr = nullptr;
            rhs.control_block = nullptr;
        }

        return *this;
    }

    ~shared_ptr(){
        if(__sync_fetch_and_sub(&control_block->counter, 1) == 1){
            control_block->destroy();
            delete control_block;
        }
    }

    pointer_type get() const {
        return ptr;
    }

    pointer_type operator->() const {
        return get();
    }

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

        virtual void destroy() = 0;
        virtual ~control_block_t(){}
    };

    template <typename U, typename Deleter>
    struct control_block_impl : control_block_t {
        U* ptr;
        Deleter deleter;

        control_block_impl(U* ptr) : ptr(ptr) {}
        control_block_impl(U* ptr, Deleter d) : ptr(ptr), deleter(d){}

        virtual void destroy(){
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

        virtual void destroy(){
            // Nothing to do
        }
    };

private:
    pointer_type ptr;
    control_block_t* control_block;
};

template <typename T, typename... Args>
std::shared_ptr<T> make_shared(Args&&... args){
    auto cb = new typename std::shared_ptr<T>::template inplace_control_block_impl<T>(std::forward<Args>(args)...);

    return std::shared_ptr<T>{&cb->ptr, cb, 0};
}

} //end of namespace std

#endif

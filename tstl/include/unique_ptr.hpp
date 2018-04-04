//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef UNIQUE_PTR_H
#define UNIQUE_PTR_H

#include <tuple.hpp>
#include <algorithms.hpp>
#include <deleter.hpp>

namespace std {

/*!
 * \brief An unique ptr of type T.
 *
 * An unique ptr represents unique ownership of dynamically allocated data.
 */
template <typename T, typename D = default_delete<T>>
struct unique_ptr {
    using pointer_type = T*; ///< The pointer type
    using element_type = T;  ///< The element type
    using deleter_type = D;  ///< The deleter type

    /*!
     * \brief Construct an empty (nullptr) unique_ptr
     */
    unique_ptr() : _data() {}

    /*!
     * \brief Construct an empty (nullptr) unique_ptr
     */
    unique_ptr(decltype(nullptr)) : unique_ptr() {}

    /*!
     * \brief Construct a new unique_ptr from the given pointer
     */
    explicit unique_ptr(pointer_type p) : _data(make_tuple(p, deleter_type())) {}

    unique_ptr(unique_ptr&& u) : _data(make_tuple(u.unlock(), u.get_deleter())) {}
    unique_ptr& operator=(unique_ptr&& u){
        reset(u.unlock());
        get_deleter() = std::forward<deleter_type>(u.get_deleter());
        return *this;
    }

    /*!
     * \brief Destructs the object and releases its memory if it still references any
     */
    ~unique_ptr(){
        reset();
    }

    // Disable copy
    unique_ptr(const unique_ptr& rhs) = delete;
    unique_ptr& operator=(const unique_ptr& rhs) = delete;

    /*!
     * \brief Assign nullptr to the unique ptr (reset it)
     */
    unique_ptr& operator=(decltype(nullptr)){
        reset();
        return *this;
    }

    //Access

    /*!
     * \brief Returns a reference to the owned object
     */
    element_type& operator*() const {
        return *get();
    }

    /*!
     * \brief Returns a pointer to the owned object
     */
    pointer_type operator->() const {
        return get();
    }

    /*!
     * \brief Returns a pointer to the owned object
     */
    pointer_type get() const {
        return std::get<0>(_data);
    }

    /*!
     * \brief Returns a reference to the deleter
     */
    deleter_type& get_deleter(){
        return std::get<1>(_data);
    }

    /*!
     * \brief Returns a const reference to the deleter
     */
    const deleter_type& get_deleter() const {
        return std::get<1>(_data);
    }

    /*!
     * \brief Converts the unique ptr to a boolean, indicating if it points to something or not
     */
    explicit operator bool() const {
        return get() == pointer_type() ? false : true;
    }

    /*!
     * \brief Extract the owned object out of the unique ptr.
     *
     * After this, the unique ptr will not own the objet anymore.
     */
    pointer_type unlock(){
        pointer_type p = get();
        std::get<0>(_data) = pointer_type();
        return p;
    }

    /*!
     * \brief Resets the unique pointer to a new state
     * \param p The new pointer value
     */
    void reset(pointer_type p = pointer_type()){
        if(get() != pointer_type()){
            get_deleter()(get());
        }

        std::get<0>(_data) = p;
    }

private:
    using data_impl = tuple<pointer_type, deleter_type>; ///< The type of internal data

    data_impl _data;  ///< The internal data storage
};

/*!
 * \brief Unique pointer implementation for an array.
 *
 * This has the same semantics, but allow random accesss as an array.
 */
template <typename T, typename D>
struct unique_ptr<T[], D> {
    using pointer_type = T*; ///< The pointer type
    using element_type = T;  ///< The element type
    using deleter_type = D;  ///< The deleter type

    /*!
     * \brief Construct an empty (nullptr) unique_ptr
     */
    unique_ptr() : _data() {}

    /*!
     * \brief Construct an empty (nullptr) unique_ptr
     */
    unique_ptr(decltype(nullptr)) : unique_ptr() {}

    /*!
     * \brief Construct a new unique_ptr from the given pointer
     */
    explicit unique_ptr(pointer_type p) : _data(make_tuple(p, deleter_type())) {}

    unique_ptr(unique_ptr&& u) : _data(make_tuple(u.unlock(), u.get_deleter())) {}
    unique_ptr& operator=(unique_ptr&& u){
        reset(u.unlock());
        get_deleter() = std::forward<deleter_type>(u.get_deleter());
        return *this;
    }

    /*!
     * \brief Destructs the object and releases its memory if it still references any
     */
    ~unique_ptr(){
        reset();
    }

    // Disable copy
    unique_ptr(const unique_ptr& rhs) = delete;
    unique_ptr& operator=(const unique_ptr& rhs) = delete;

    unique_ptr& operator=(decltype(nullptr)){
        reset();
        return *this;
    }

    /*!
     * \brief Returns a pointer to the owned object
     */
    pointer_type get() const {
        return std::get<0>(_data);
    }

    /*!
     * \brief Returns a reference to the deleter
     */
    deleter_type& get_deleter(){
        return std::get<1>(_data);
    }

    /*!
     * \brief Returns a const reference to the deleter
     */
    const deleter_type& get_deleter() const {
        return std::get<1>(_data);
    }

    /*!
     * \brief Returns a reference to an element of the array
     * \param i The index inside the array
     */
    element_type& operator[](size_t i) const {
        return get()[i];
    }

    /*!
     * \brief Converts the unique ptr to a boolean, indicating if it points to something or not
     */
    explicit operator bool() const {
        return get() == pointer_type() ? false : true;
    }

    /*!
     * \brief Extract the owned object out of the unique ptr.
     *
     * After this, the unique ptr will not own the objet anymore.
     */
    pointer_type unlock(){
        pointer_type p = get();
        std::get<0>(_data) = pointer_type();
        return p;
    }

    /*!
     * \brief Resets the unique pointer to an empty state
     */
    void reset(){
        reset(pointer_type());
    }

    /*!
     * \brief Resets the unique pointer to a new state
     * \param p The new pointer value
     */
    void reset(pointer_type p){
        auto tmp = get();
        std::get<0>(_data) = p;
        if(tmp){
            get_deleter()(tmp);
        }
    }

private:
    using data_impl = tuple<pointer_type, deleter_type>; ///< The type of internal data

    data_impl _data;  ///< The internal data storage
};

static_assert(sizeof(unique_ptr<long>) == sizeof(long), "unique_ptr must have zero overhead with default deleter");
static_assert(sizeof(unique_ptr<long[]>) == sizeof(long), "unique_ptr must have zero overhead with default deleter");

/*!
 * \brief Helper to create an unique_ptr from the args
 */
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args){
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} //end of namespace std

#endif

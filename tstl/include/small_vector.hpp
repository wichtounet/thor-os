//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef SMALL_VECTOR_H
#define SMALL_VECTOR_H

#include <types.hpp>
#include <algorithms.hpp>
#include <vector.hpp>
#include <unique_ptr.hpp>
#include <iterator.hpp>

namespace std {

template<typename T>
struct vector_base_long {
    size_t capacity;
    T* data;

    vector_base_long() = default;

    vector_base_long(size_t capacity, T* array) : capacity(capacity), data(array) {}

    vector_base_long(vector_base_long&) = delete;
    vector_base_long& operator=(vector_base_long&) = delete;

    vector_base_long(vector_base_long&&) = default;
    vector_base_long& operator=(vector_base_long&&) = default;
};

static constexpr const size_t vector_min_capacity = 16;
static constexpr const size_t vector_words = vector_min_capacity / sizeof(size_t);

template<typename T>
struct vector_base_small {
    static constexpr size_t min_capacity = vector_min_capacity / sizeof(T);

    T data[vector_min_capacity];

    vector_base_small() = default;

    vector_base_small(vector_base_small&) = delete;
    vector_base_small& operator=(vector_base_small&) = delete;

    vector_base_small(vector_base_small&&) = default;
    vector_base_small& operator=(vector_base_small&&) = default;
};

struct vector_base_raw {
    size_t data[vector_words];

    vector_base_raw() = delete;

    vector_base_raw(vector_base_raw&) = delete;
    vector_base_raw& operator=(vector_base_raw&) = delete;

    vector_base_raw(vector_base_raw&&) = delete;
    vector_base_raw& operator=(vector_base_raw&&) = delete;
};

template<typename T>
union vector_base_storage {
    vector_base_small<T> small;
    vector_base_long<T> big;
    vector_base_raw raw;

    vector_base_storage(){
        //Default construction: Nothing to do
    }

    ~vector_base_storage() {}
};

static_assert(vector_min_capacity == sizeof(vector_base_small<char>), "vector_base_small must be the correct SSO size");
static_assert(vector_min_capacity == sizeof(vector_base_long<char>), "vector_base_long must be the correct SSO size");
static_assert(vector_min_capacity == sizeof(vector_base_raw), "vector_base_raw must be the correct SSO size");

/*!
 * \brief A string of the given character type.
 *
 * This implementation uses SSO to not allocate any dynamic memory on short strings (<16 chars)
 */
template<typename T>
struct small_vector {
    using value_type           = T;                 ///< The value type contained in the vector
    using pointer_type         = value_type*;       ///< The pointer type contained in the vector
    using reference_type       = value_type&;       ///< The pointer type contained in the vector
    using const_reference_type = const value_type&; ///< The pointer type contained in the vector
    using size_type            = size_t;            ///< The size type
    using iterator             = value_type*;       ///< The iterator type
    using const_iterator       = const value_type*; ///< The const iterator type

    using reverse_iterator       = std::reverse_iterator<iterator>;       ///< The reverse iterator type
    using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< The const reverse iterator type

    static constexpr size_t small_capacity = vector_min_capacity / sizeof(T);

private:
    size_t _size;

    vector_base_storage<T> storage;

    void set_long(bool small){
        if(small){
            _size |= (1UL << 63);
        } else {
            _size &= ~(1UL << 63);
        }
    }

    void set_small(bool small){
        set_long(!small);
    }

    void set_size(size_t size){
        if(is_long()){
            _size = size | (1UL << 63);
        } else {
            _size = size;
        }
    }

    bool is_long() const {
        return _size & (1UL << 63);
    }

    bool is_small() const {
        return !is_long();
    }

    void zero(){
        for(size_t i = 0; i < vector_words; ++i){
            storage.raw.data[i] = 0;
        }
    }

public:
    //Constructors

    /*!
     * \brief Construct an empty vector
     */
    small_vector() : _size(0){
        set_small(true);
    }

    /*!
     * \brief Construct a new vector of the given capacity.
     */
    explicit small_vector(size_t __size, T value = T()) : _size(__size) {
        set_small(__size <= small_capacity);

        if(!is_small()){
            new (&storage.big) vector_base_long<T>(__size, allocate(__size));
        }

        // Fill with the default value
        for(size_t i = 0; i < __size; ++i){
            new (&data_ptr()[i]) value_type(value);
        }
    }

    /*!
     * \brief Construct a vector containing the given values
     */
    small_vector(initializer_list<T> values) : _size(values.size()) {
        set_small(values.size() <= small_capacity);

        if(!is_small()){
            new (&storage.big) vector_base_long<T>(size(), allocate(size()));
        }

        size_t i = 0;
        for(auto& v : values){
            new (&data_ptr()[i++]) value_type(v);
        }
    }

    /*!
     * \brief Construct a new string from the given range of characters
     */
    template <typename It>
    small_vector(It it, It end) {
        _size = std::distance(it, end);

        auto capacity = size() + 1;

        set_small(capacity <= small_capacity);

        if(!is_small()){
            new (&storage.big) vector_base_long<T>(capacity, allocate(capacity));
        }

        auto oit = begin();
        while(it != end){
            *oit++ = *it++;
        }

        (*this)[size()] = '\0';
    }

    //Copy

    small_vector(const small_vector& rhs) : _size(rhs._size) {
        if(!is_small()){
            new (&storage.big) vector_base_long<T>(size() + 1, allocate(size() + 1));
        }

            // Copy the new values
        for(size_t i = 0; i < size(); ++i){
            new (&data_ptr()[i]) value_type(rhs.data_ptr()[i]);
        }
    }

    small_vector& operator=(const small_vector& rhs){
        if(this != &rhs){
            set_size(rhs.size());

            // We need to destruct the previous element
            release();

            if(capacity() < rhs.capacity()){
                auto capacity = rhs.capacity();

                if(is_small()){
                    new (&storage.big) vector_base_long<T>(capacity, allocate(capacity));

                    set_small(false);
                } else {
                    storage.big.capacity = capacity;
                    storage.big.data = allocate(capacity);
                }
            }

            // Copy the new values
            for (size_t i = 0; i < size(); ++i) {
                new (&data_ptr()[i]) value_type(rhs.data_ptr()[i]);
            }
        }

        return *this;
    }

    //Move

    small_vector(small_vector&& rhs) : _size(rhs._size) {
        if(is_small()){
            new (&storage.small) vector_base_small<T>(std::move(rhs.storage.small));
        } else {
            new (&storage.big) vector_base_long<T>(std::move(rhs.storage.big));
        }

        rhs.set_size(0);
        rhs.zero();
    }

    small_vector& operator=(small_vector&& rhs){
        auto was_small = is_small();
        auto was_long = !was_small;

        auto small = rhs.is_small();
        auto lng = !small;

        set_size(rhs.size());

        if(was_small && small){
            storage.small = std::move(rhs.storage.small);
        } else if(was_long && lng){
            storage.big = std::move(rhs.storage.big);
        } else if(was_small && lng){
            new (&storage.big) vector_base_long<T>(std::move(rhs.storage.big));

            set_small(false);
        } else if(was_long && small){
            ensure_capacity(rhs.size() + 1);

            std::move_n(rhs.begin(), size() + 1, begin());
        }

        rhs.set_size(0);
        rhs.zero();

        return *this;
    }

    //Destructors

    /*!
     * \brief Destructs the string and releases all its associated memory
     */
    ~small_vector(){
        release();
    }

    //Modifiers

    void adjust_size(size_t size){
        set_size(size);
    }

    /*!
     * \brief Clear the string
     */
    void clear(){
        destruct_all();

        set_size(0);
    }

    /*!
     * \brief Pop the last character of the string.
     */
    void pop_back(){
        set_size(size() - 1);

        // Call the destructor of the erased value
        data_ptr()[size()].~value_type();
    }

    /*!
     * \brief Ensures a capacity of at least new_capacity
     */
    void reserve(size_t new_capacity){
        ensure_capacity(new_capacity);
    }

    /*!
     * \brief Resize the vector to the given size
     */
    void resize(size_t new_size){
        if(new_size > size()){
            ensure_capacity(new_size);

            // Default initialize the new elements
            for(size_t i = size(); i < new_size; ++i){
                new (&data_ptr()[i]) value_type();
            }

            set_size(new_size);
        } else if(new_size < size()){
            // Call the necessary destructors
            for(size_t i = new_size; i < size(); ++i){
                data_ptr()[i].~value_type();
            }

            //By diminishing the size, the last elements become unreachable
            set_size(new_size);
        }
    }

    /*!
     * \brief Add an element at the back of the vector
     */
    void push_back(value_type&& element){
        const auto old_size = size();

        ensure_capacity(old_size + 1);

        new (&data_ptr()[old_size]) value_type(std::move(element));

        set_size(old_size + 1);
    }

    /*!
     * \brief Add an element at the back of the vector
     */
    void push_back(const value_type& element){
        const auto old_size = size();

        ensure_capacity(old_size + 1);

        new (&data_ptr()[old_size]) value_type(element);

        set_size(old_size + 1);
    }

    /*!
     * \brief Construct a new element inplace
     */
    value_type& emplace_back(){
        const auto old_size = size();

        ensure_capacity(old_size + 1);

        new (&data_ptr()[old_size]) value_type();

        set_size(old_size + 1);

        return back();
    }

    /*!
     * \brief Construct a new element inplace
     */
    template<typename... Args>
    value_type& emplace_back(Args... args){
        const auto old_size = size();

        ensure_capacity(old_size + 1);

        new (&data_ptr()[old_size]) value_type{std::forward<Args>(args)...};

        set_size(old_size + 1);

        return back();
    }

    /*!
     * \brief Add an element at the front of the vector
     */
    void push_front(value_type&& element){
        const auto old_size = size();

        ensure_capacity(old_size + 1);

        if(!empty()){
            new (&data_ptr()[old_size]) value_type(std::move(data_ptr()[old_size - 1]));

            for (size_t i = old_size - 1; i > 0; --i) {
                data_ptr()[i] = std::move(data_ptr()[i - 1]);
            }
        }

        // At this point _data[0] has been deleted
        data_ptr()[0] = std::move(element);

        set_size(old_size + 1);
    }

    /*!
     * \brief Add an element at the front of the vector
     */
    void push_front(const value_type& element){
        const auto old_size = size();

        ensure_capacity(old_size + 1);

        if(!empty()){
            new (&data_ptr()[old_size]) value_type(std::move(data_ptr()[old_size - 1]));

            for (size_t i = old_size - 1; i > 0; --i) {
                data_ptr()[i] = std::move(data_ptr()[i - 1]);
            }
        }

        // At this point _data[0] has been deleted
        data_ptr()[0] = element;

        set_size(old_size + 1);
    }

    /*!
     * \brief Erase the element at the given position
     */
    void erase(size_t position){
        for(size_t i = position; i < size() - 1; ++i){
            data_ptr()[i] = std::move(data_ptr()[i+1]);
        }

        set_size(size() - 1);

        // Call the destructor of the last value
        data_ptr()[size()].~value_type();
    }

    /*!
     * \brief Erase the element at the given position
     */
    void erase(iterator position){
        for(size_t i = position - begin(); i < size() - 1; ++i){
            data_ptr()[i] = std::move(data_ptr()[i+1]);
        }

        set_size(size() - 1);

        // Call the destructor of the last value
        data_ptr()[size()].~value_type();
    }

    /*!
     * \brief Erase all the elements of the given range
     */
    void erase(iterator first, iterator last){
        auto n = std::distance(first, last);

        for(size_t i = first - begin(); i < size() - n; ++i){
            data_ptr()[i] = std::move(data_ptr()[i+n]);
        }

        // Call the destructors on the erase elements
        for(size_t i = size() - n; i < size(); ++i){
            data_ptr()[i].~value_type();
        }

        set_size(size() - n);
    }

    //Accessors

    /*!
     * \brief Returns the size of the string
     */
    size_t size() const {
        return _size & ~(1UL << 63);
    }

    /*!
     * \brief Returns the capacity of the string
     */
    size_t capacity() const {
        if(is_small()){
            return small_capacity;
        } else {
            return storage.big.capacity;
        }
    }

    /*
     * \brief Indicates if the string is empty
     */
    bool empty() const {
        return size() == 0;
    }

    T* data_ptr(){
        if(is_small()){
            return &storage.small.data[0];
        } else {
            return &storage.big.data[0];
        }
    }

    const T* data_ptr() const {
        if(is_small()){
            return &storage.small.data[0];
        } else {
            return &storage.big.data[0];
        }
    }

    /*!
     * \brief Returns a reference to the ith character
     */
    T& operator[](size_t i){
        return data_ptr()[i];
    }

    /*!
     * \brief Returns a const reference to the ith character
     */
    const T& operator[](size_t i) const {
        return data_ptr()[i];
    }

    /*!
     * \brief Returns a reference to the element at the front of the collection
     */
    value_type& front(){
        return data_ptr()[0];
    }

    /*!
     * \brief Returns a const reference to the element at the front of the collection
     */
    const value_type& front() const  {
        return data_ptr()[0];
    }

    /*!
     * \brief Returns a reference to the element at the back of the collection
     */
    value_type& back(){
        return data_ptr()[size() - 1];
    }

    /*!
     * \brief Returns a const reference to the element at the back of the collection
     */
    const value_type& back() const  {
        return data_ptr()[size() - 1];
    }

    //Operators

    /*!
     * \brief Test if this string is equal to the given string
     */
    bool operator==(const small_vector& rhs) const {
        if(size() != rhs.size()){
            return false;
        }

        for(size_t i = 0; i < size(); ++i){
            if((*this)[i] != rhs[i]){
                return false;
            }
        }

        return true;
    }

    /*!
     * \brief Test if this string is not equal to the given string
     */
    bool operator!=(const small_vector& rhs) const {
        return !(*this == rhs);
    }

    //Iterators

    /*!
     * \brief Returns an iterator to the first character of the string
     */
    iterator begin(){
        return iterator(&data_ptr()[0]);
    }

    /*!
     * \brief Returns a const iterator to the first character of the string
     */
    const_iterator begin() const {
        return const_iterator(&data_ptr()[0]);
    }

    /*!
     * \brief Returns an iterator the past-the-end character of the string
     */
    iterator end(){
        return iterator(&data_ptr()[size()]);
    }

    /*!
     * \brief Returns a const iterator the past-the-end character of the string
     */
    const_iterator end() const {
        return const_iterator(&data_ptr()[size()]);
    }

    // Reverse Iterators

    /*!
     * \brief Return a reverse iterator to point to the first element
     */
    reverse_iterator rbegin(){
        return reverse_iterator(&data_ptr()[int64_t(size()) - 1]);
    }

    /*!
     * \brief Return a reverse iterator to point to the first element
     */
    constexpr const_reverse_iterator rbegin() const {
        return const_iterator(&data_ptr()[int64_t(size()) - 1]);
    }

    /*!
     * \brief Return a reverse iterator point to the past-the-end element
     */
    reverse_iterator rend(){
        return reverse_iterator(&data_ptr()[-1]);
    }

    /*!
     * \brief Return a reverse iterator point to the past-the-end element
     */
    constexpr const_reverse_iterator rend() const {
        return const_reverse_iterator(&data_ptr()[-1]);
    }

private:
    static value_type* allocate(size_t n){
        return reinterpret_cast<value_type*>(new uint8_t[n * sizeof(value_type)]);
    }

    static void deallocate(value_type* ptr){
        delete[] reinterpret_cast<uint8_t*>(ptr);
    }

    void destruct_all(){
        // Call the destructors
        for(size_t i = 0; i < size(); ++i){
            data_ptr()[i].~value_type();
        }
    }

    void release(){
        destruct_all();

        if(!is_small()){
            // Deallocate the memory
            deallocate(storage.big.data);
            storage.big.data = nullptr;
        }
    }

    void ensure_capacity(size_t new_capacity){
        // Note: We know that min capacity is constant, therefore
        // augmenting the capacity always means going to long
        // storage
        if(new_capacity > 0 && (capacity() < new_capacity)){
            auto new_cap = capacity() * 2;

            if(new_cap < new_capacity){
                new_cap = new_capacity;
            }

            auto new_data = allocate(new_cap);

            // Move the old _data into the new one
            for(size_t i = 0; i < size(); ++i){
                new (&new_data[i]) value_type(std::move(data_ptr()[i]));
            }

            if(is_small()){
                new (&storage.big) vector_base_long<T>(new_cap, new_data);

                set_small(false);
            } else {
                release();

                storage.big.data = new_data;
                storage.big.capacity = new_cap;
            }
        }
    }
};

} //end of namespace std

#endif

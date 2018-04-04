//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef DEQUE_H
#define DEQUE_H

#include <initializer_list.hpp>
#include <types.hpp>
#include <algorithms.hpp>
#include <new.hpp>
#include <iterator.hpp>

namespace std {

template <typename T>
struct deque;

template <typename T>
struct deque_iterator {
    using iterator_type   = deque_iterator<T>; ///< The iterator type
    using value_type      = T;                 ///< The value type
    using difference_type = int64_t;           ///< The difference type
    using pointer         = value_type*;       ///< The pointer type
    using reference       = value_type&;       ///< The reference type

    deque_iterator(deque<T>* container, int64_t index)
            : container(container), index(index) {
        // Nothing else to init
    }

    deque_iterator(const deque_iterator& rhs) : container(rhs.container), index(rhs.index) {
        // Nothing else to init
    }

    deque_iterator& operator=(const deque_iterator& rhs){
        if(this != &rhs){
            this->container = rhs.container;
            this->index = rhs.index;
        }

        return *this;
    }

    T& operator*() {
        return (*container)[index];
    }

    deque_iterator& operator++() {
        ++index;
        return *this;
    }

    deque_iterator operator++(int) {
        auto it = *this;
        ++(*this);
        return it;
    }

    deque_iterator& operator--() {
        --index;
        return *this;
    }

    deque_iterator operator--(int) {
        auto it = *this;
        --(*this);
        return it;
    }

    deque_iterator& operator+=(int n) {
        index += n;
        return *this;
    }

    deque_iterator operator+(int n) {
        auto it = *this;
        it += n;
        return it;
    }

    deque_iterator& operator-=(int n) {
        index -= n;
        return *this;
    }

    deque_iterator operator-(int n) {
        auto it = *this;
        it -= n;
        return it;
    }

    bool operator==(const deque_iterator& rhs) {
        return index == rhs.index;
    }

    bool operator!=(const deque_iterator& rhs) {
        return index != rhs.index;
    }

    difference_type operator-(const deque_iterator& rhs) {
        return index - rhs.index;
    }

    friend struct deque<T>;

private:
    deque<T>* container;
    int64_t index;
};

// TODO We should find a way to get rid of the painful situation when size
// = 0 for the indices

/*!
 * \brief A double-ended queue.
 *
 * Insertions at the front and at the back are done in O(1) and random access is
 * possible in O(1). Insertions at other positions is done in O(n).
 */
template <typename T>
struct deque {
    using value_type           = T;                 ///< The value type contained in the vector
    using pointer_type         = value_type*;       ///< The pointer type contained in the vector
    using size_type            = size_t;            ///< The size type
    using reference_type       = value_type&;       ///< The reference type
    using const_reference_type = const value_type&; ///< The const reference type

    using iterator       = deque_iterator<T>;       ///< The iterator type
    using const_iterator = deque_iterator<const T>; ///< The const iterator type

    using reverse_iterator       = std::reverse_iterator<iterator>;       ///< The reverse iterator type
    using const_reverse_iterator = std::reverse_iterator<const_iterator>; ///< The const reverse iterator type

    static constexpr const size_t block_elements = 16;                         ///< Number of entries per block
    static constexpr const size_t block_size     = block_elements * sizeof(T); ///< Size of a block, in bytes

    /*!
     * \brief Construct an empty deque
     */
    deque()
            : data(nullptr), first_element(0), last_element(0), blocks(0), _size(0) {
        // By default, no capacity
    }

    /*!
     * \brief Construct a deque containing the given values
     */
    deque(initializer_list<T> values)
            : deque() {
        ensure_capacity_back(values.size());

        for (auto& value : values) {
            push_back(value);
        }
    }

    //TODO
    deque(const deque& rhs) = delete;
    deque& operator=(const deque& rhs) = delete;

    deque(deque&& rhs) : data(rhs.data), first_element(rhs.first_element), last_element(rhs.last_element), blocks(rhs.blocks), _size(rhs._size){
        rhs.data          = nullptr;
        rhs.first_element = 0;
        rhs.last_element  = 0;
        rhs.blocks        = 0;
        rhs._size         = 0;
    }

    deque& operator=(deque&& rhs){
        if(this != &rhs){
            this->data          = rhs.data;
            this->first_element = rhs.first_element;
            this->last_element  = rhs.last_element;
            this->blocks        = rhs.blocks;
            this->_size         = rhs._size;

            rhs.data          = nullptr;
            rhs.first_element = 0;
            rhs.last_element  = 0;
            rhs.blocks        = 0;
            rhs._size         = 0;
        }

        return *this;
    }

    void destruct_all(){
        for (size_t i = 0; i < _size; ++i) {
            (*this)[i].~value_type();
        }
    }

    ~deque() {
        destruct_all();

        for(size_t i = 0; i < blocks; ++i){
            deallocate(data[i]);
        }

        delete[] data;
    }

    /*!
     * \brief Push an element at the back of the deque
     */
    void push_back(T&& element) {
        ensure_capacity_back(1);

        if (_size) {
            auto block = (last_element + 1) / block_elements;
            auto index = (last_element + 1) % block_elements;

            new (&data[block][index]) T(std::move(element));

            ++last_element;
        } else {
            auto block = last_element / block_elements;
            auto index = last_element % block_elements;

            new (&data[block][index]) T(std::move(element));
        }

        ++_size;
    }

    /*!
     * \brief Push an element at the back of the deque
     */
    void push_back(const T& element) {
        ensure_capacity_back(1);

        if (_size) {
            auto block = (last_element + 1) / block_elements;
            auto index = (last_element + 1) % block_elements;

            new (&data[block][index]) T(element);

            ++last_element;
        } else {
            auto block = last_element / block_elements;
            auto index = last_element % block_elements;

            new (&data[block][index]) T(element);
        }

        ++_size;
    }

    /*!
     * \brief Construct an element at the back of the vector, enplace.
     */
    reference_type emplace_back() {
        ensure_capacity_back(1);

        if (_size) {
            auto block = (last_element + 1) / block_elements;
            auto index = (last_element + 1) % block_elements;

            new (&data[block][index]) T();

            ++last_element;
        } else {
            auto block = last_element / block_elements;
            auto index = last_element % block_elements;

            new (&data[block][index]) T();
        }

        ++_size;

        return back();
    }

    /*!
     * \brief Construct an element at the back of the vector, enplace.
     */
    template <typename... Args>
    reference_type emplace_back(Args&&... args) {
        ensure_capacity_back(1);

        if (_size) {
            auto block = (last_element + 1) / block_elements;
            auto index = (last_element + 1) % block_elements;

            new (&data[block][index]) T(std::forward<Args>(args)...);

            ++last_element;
        } else {
            auto block = last_element / block_elements;
            auto index = last_element % block_elements;

            new (&data[block][index]) T(std::forward<Args>(args)...);
        }

        ++_size;

        return back();
    }

    /*!
     * \brief Push an element at the front of the deque
     */
    void push_front(T&& element) {
        ensure_capacity_front(1);

        if (_size) {
            auto block = (first_element - 1) / block_elements;
            auto index = (first_element - 1) % block_elements;

            new (&data[block][index]) T(std::move(element));

            --first_element;
        } else {
            auto block = first_element / block_elements;
            auto index = first_element % block_elements;

            new (&data[block][index]) T(std::move(element));
        }

        ++_size;
    }

    /*!
     * \brief Push an element at the front of the deque
     */
    void push_front(const T& element) {
        ensure_capacity_front(1);

        if (_size) {
            auto block = (first_element - 1) / block_elements;
            auto index = (first_element - 1) % block_elements;

            new (&data[block][index]) T(element);

            --first_element;
        } else {
            auto block = first_element / block_elements;
            auto index = first_element % block_elements;

            new (&data[block][index]) T(element);
        }

        ++_size;
    }

    /*!
     * \brief Construct an element at the front of the vector, enplace.
     */
    reference_type emplace_front() {
        ensure_capacity_front(1);

        if (_size) {
            auto block = (first_element - 1) / block_elements;
            auto index = (first_element - 1) % block_elements;

            new (&data[block][index]) T();

            --first_element;
        } else {
            auto block = first_element / block_elements;
            auto index = first_element % block_elements;

            new (&data[block][index]) T();
        }

        ++_size;
    }

    /*!
     * \brief Construct an element at the front of the vector, enplace.
     */
    template <typename... Args>
    reference_type emplace_front(Args&&... args) {
        ensure_capacity_front(1);

        if (_size) {
            auto block = (first_element - 1) / block_elements;
            auto index = (first_element - 1) % block_elements;

            new (&data[block][index]) T(std::forward<Args>(args)...);

            --first_element;
        } else {
            auto block = first_element / block_elements;
            auto index = first_element % block_elements;

            new (&data[block][index]) T(std::forward<Args>(args)...);
        }

        ++_size;
    }

    /*!
     * \brief Removes an element from the back of the deque
     */
    void pop_back() {
        auto block = (last_element) / block_elements;
        auto index = (last_element) % block_elements;
        data[block][index].~value_type();

        --_size;
        --last_element;

        if (!_size) {
            first_element = last_element = 0;
        }
    }

    /*!
     * \brief Removes an element from the front of the deque
     */
    void pop_front() {
        auto block = (first_element) / block_elements;
        auto index = (first_element) % block_elements;
        data[block][index].~value_type();

        --_size;
        ++first_element;

        if (!_size) {
            first_element = last_element = 0;
        }
    }

    /*!
     * \brief Erase the element at the given position
     */
    void erase(size_t position) {
        for (size_t i = position; i < _size - 1; ++i) {
            (*this)[i] = std::move((*this)[i + 1]);
        }

        auto block = (last_element) / block_elements;
        auto index = (last_element) % block_elements;
        data[block][index].~value_type();

        --last_element;
        --_size;
    }

    /*!
     * \brief Erase the element at the given position
     */
    iterator erase(iterator position) {
        erase(position.index);

        return position;
    }

    /*!
     * \brief Erase all the elements of the given range
     */
    void erase(iterator first, iterator last) {
        auto n = last - first;

        for (size_t i = first.index; i < _size - n; ++i) {
            (*this)[i] = std::move((*this)[i + n]);
        }

        for (size_t i = _size - n; i < _size; ++i) {
            (*this)[i].~value_type();
        }

        _size -= n;
        last_element -= n;
    }

    /*!
     * \brief Removes all the elements of the deque
     */
    void clear() {
        destruct_all();

        first_element = 0;
        last_element  = 0;

        _size = 0;
    }

    /*!
     * \brief Returns the size of the deque
     */
    size_t size() const {
        return _size;
    }

    /*!
     * \brief Returns the current maximum size of the deque
     */
    size_t max_size() const {
        return blocks * block_elements;
    }

    /*!
     * \brief Retuns a reference to the element at position i
     */
    reference_type operator[](size_t i) {
        auto block = (i + first_element) / block_elements;
        auto index = (i + first_element) % block_elements;
        return data[block][index];
    }

    /*!
     * \brief Retuns a reference to the element at position i
     */
    const_reference_type operator[](size_t i) const {
        auto block = (i + first_element) / block_elements;
        auto index = (i + first_element) % block_elements;
        return data[block][index];
    }

    /*!
     * \brief Returns a reference to the back element of the deque
     */
    reference_type back() {
        auto block = (last_element) / block_elements;
        auto index = (last_element) % block_elements;
        return data[block][index];
    }

    /*!
     * \brief Returns a reference to the back element of the deque
     */
    const_reference_type back() const {
        auto block = (last_element) / block_elements;
        auto index = (last_element) % block_elements;
        return data[block][index];
    }

    /*!
     * \brief Returns a reference to the front element of the deque
     */
    reference_type front() {
        auto block = (first_element) / block_elements;
        auto index = (first_element) % block_elements;
        return data[block][index];
    }

    /*!
     * \brief Returns a reference to the front element of the deque
     */
    const_reference_type front() const {
        auto block = (first_element) / block_elements;
        auto index = (first_element) % block_elements;
        return data[block][index];
    }

    //Iterators

    /*!
     * \brief Return an iterator to point to the first element
     */
    iterator begin() {
        return iterator(this, 0);
    }

    /*!
     * \brief Return an iterator to point to the first element
     */
    constexpr const_iterator begin() const {
        return const_iterator(this, 0);
    }

    /*!
     * \brief Return an iterator to point to the past-the-end element
     */
    iterator end() {
        return iterator(this, _size);
    }

    /*!
     * \brief Return an iterator to point to the past-the-end element
     */
    constexpr const_iterator end() const {
        return const_iterator(this, _size);
    }

    //Iterators

    /*!
     * \brief Return a reverse iterator to point to the first element
     */
    reverse_iterator rbegin() {
        return reverse_iterator(iterator(this, _size - 1));
    }

    /*!
     * \brief Return a reverse iterator to point to the first element
     */
    constexpr const_reverse_iterator rbegin() const {
        return const_iterator(const_iterator(this, _size - 1));
    }

    /*!
     * \brief Return a reverse iterator point to the past-the-end element
     */
    reverse_iterator rend() {
        return reverse_iterator(iterator(this, -1));
    }

    /*!
     * \brief Return a reverse iterator point to the past-the-end element
     */
    constexpr const_reverse_iterator rend() const {
        return const_reverse_iterator(const_iterator(this, -1));
    }

private:
    static value_type* allocate(size_t n){
        return reinterpret_cast<value_type*>(new char[n * sizeof(value_type)]);
    }

    static void deallocate(value_type* ptr){
        delete[] reinterpret_cast<char*>(ptr);
    }

    void ensure_capacity_front(size_t n) {
        auto capacity_front = first_element;

        if (capacity_front < n) {
            if (!data) {
                blocks = n % block_elements == 0 ? n / block_elements : (n / block_elements) + 1;

                data = new T*[blocks];

                for (size_t i = 0; i < blocks; ++i) {
                    data[i] = allocate(block_elements);
                }

                first_element = blocks * block_elements - 1;
                last_element  = blocks * block_elements - 1;
            } else {
                auto expand_front = n - capacity_front;
                auto new_blocks   = expand_front % block_elements == 0 ? expand_front / block_elements : (expand_front / block_elements) + 1;

                auto new_data = new T*[blocks + new_blocks];

                for (size_t i = 0; i < blocks; ++i) {
                    new_data[i + new_blocks] = data[i];
                }

                for (size_t i = 0; i < new_blocks; ++i) {
                    new_data[i] = allocate(block_elements);
                }

                first_element += new_blocks * block_elements;
                last_element += new_blocks * block_elements;

                delete[] data;
                data = new_data;
                blocks += new_blocks;
            }
        }
    }

    void ensure_capacity_back(size_t n) {
        auto very_last_element = blocks ? blocks * block_elements - 1 : 0;
        auto capacity_back     = very_last_element - last_element;

        if (capacity_back < n) {
            if (!data) {
                blocks = n % block_elements == 0 ? n / block_elements : (n / block_elements) + 1;

                data = new T*[blocks];

                for (size_t i = 0; i < blocks; ++i) {
                    data[i] = allocate(block_elements);
                }

                first_element = 0;
                last_element  = 0;
            } else {
                auto expand_back = n - capacity_back;
                auto new_blocks  = expand_back % block_elements == 0 ? expand_back / block_elements : (expand_back / block_elements) + 1;

                auto new_data = new T*[blocks + new_blocks];

                for (size_t i = 0; i < blocks; ++i) {
                    new_data[i] = data[i];
                }

                for (size_t i = blocks; i < blocks + new_blocks; ++i) {
                    new_data[i] = allocate(block_elements);
                }

                delete[] data;
                data = new_data;
                blocks += new_blocks;
            }
        }
    }

    T** data;
    size_t first_element;
    size_t last_element;
    size_t blocks;
    size_t _size;
};

} //end of namespace std

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef VFS_PATH_H
#define VFS_PATH_H

#include <vector.hpp>
#include <string.hpp>

/*!
 * \brief Structure to represent a path on the file system
 */
struct path {
    typedef std::vector<std::string>::const_iterator iterator; ///< The type of iterator

    /*!
     * \brief Construct an empty path.
     *
     * Such path is invalid
     */
    path();

    /*!
     * \brief Construct a path from a string
     */
    path(const std::string& path);

    /*!
     * \brief Construct a path by concatenating a base path and a string path
     */
    path(const path& base_path, const std::string& path);

    /*!
     * \brief Construct a path by concatenating a base path and a second path
     */
    path(const path& base_path, const path& p);

    path(const path&) = default;
    path(path&&) = default;

    path& operator=(const path&) = default;
    path& operator=(path&&) = default;

    // Conversion functions

    /*!
     * \brief Returns a string  representation of the path
     */
    std::string string() const;

    /*!
     * \brief Returns a reference to the internal representation of the path
     */
    const std::vector<std::string>& vec() const;

    // Modifiers

    /*
     * \brief Makes the path invalid
     */
    void invalidate();

    // Accessors

    /*!
     * \brief Returns true if the path is empty.
     *
     * Such a path will be invalid
     */
    bool empty() const;

    /*!
     * \brief Returns true if the path points to the root directory, false
     * otherwise
     */
    bool is_root() const;

    /*!
     * \brief Returns true if the path is valid, false otherwise
     */
    bool is_valid() const;

    /*!
     * \brief Returns true if the path is absolute and has only 2 components.
     */
    bool is_sub_root() const;

    /*!
     * \brief Returns the number of elements of the path
     */
    size_t size() const;

    /*!
     * \brief Returns the base name (the last part)
     */
    std::string base_name() const;

    /*!
     * \brief Returns the root name (the first part)
     */
    std::string root_name() const;

    /*!
     * \brief Returns the sub root name (the second part)
     */
    std::string sub_root_name() const;

    /*!
     * \brief Returns true if the path is absoluate, false otherwise
     */
    bool is_absolute() const;

    /*!
     * \brief Returns true if the path is relaative, false otherwise
     */
    bool is_relative() const;

    // Accessors to sub parts

    /*!
     * \brief Returns the ith part of the path
     */
    const std::string& name(size_t i) const;

    /*!
     * \brief Returns the ith part of the path
     */
    const std::string& operator[](size_t i) const;

    // Decomposition functions

    /*!
     * \brief Returns the path minus the i first elements
     */
    path sub_path(size_t i) const;

    /*!
     * \brief Returns the path minus the last element
     */
    path branch_path() const;

    // Iterators

    /*!
     * \brief Returns an iterator poiting to the first element.
     */
    iterator begin() const ;

    /*!
     * \brief Returns an iterator poiting past the end element.
     */
    iterator end() const ;

    // relational operators

    /*!
     * \brief Returns true if the two paths are equivalent
     */
    bool operator==(const path& p) const;

    /*!
     * \brief Returns true if the two paths are not equivalent
     */
    bool operator!=(const path& p) const;

    private:
        std::vector<std::string> names;
};

/*!
 * \brief Form a new path by concatenation of both paths
 */
path operator/(const path& lhs, const path& rhs);

/*!
 * \brief Form a new path by concatenation of a path and a string.
 */
path operator/(const path& lhs, const std::string& rhs);

/*!
 * \brief Form a new path by concatenation of a string and a path.
 */
path operator/(const std::string& lhs, const path& rhs);

#endif

//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VFS_PATH_H
#define VFS_PATH_H

#include <vector.hpp>
#include <string.hpp>

/*!
 * \brief Structure to represent a path on the file system
 */
struct path {
    typedef std::vector<std::string>::const_iterator iterator;

    path();
    path(const std::string& path);
    path(const path& base_path, const std::string& path);
    path(const path& base_path, const path& p);

    path(const path&) = default;
    path(path&&) = default;

    path& operator=(const path&) = default;
    path& operator=(path&&) = default;

    // Conversion functions
    std::string string() const;
    const std::vector<std::string>& vec() const;

    // Modifiers
    void invalidate();

    // Accessors
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

    bool is_sub_root() const;
    size_t size() const;
    std::string base_name() const;
    std::string root_name() const;
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
    const std::string& name(size_t i) const;
    const std::string& operator[](size_t i) const;

    // Decomposition functions
    path sub_path(size_t i) const;
    path branch_path() const;

    // Iterators
    iterator begin() const ;
    iterator end() const ;

    // relational operators
    bool operator==(const path& p) const;
    bool operator!=(const path& p) const;

    private:
        std::vector<std::string> names;
};

path operator/(const path& lhs, const path& rhs);
path operator/(const path& lhs, const std::string& rhs);
path operator/(const std::string& lhs, const path& rhs);

#endif

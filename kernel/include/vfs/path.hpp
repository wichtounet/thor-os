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

    path(const path&) = default;
    path(path&&) = default;

    path& operator=(const path&) = default;
    path& operator=(path&&) = default;

    // Conversion functions
    std::string string() const;

    // Accessors
    bool empty() const;

    // Iterators
    iterator begin() const ;
    iterator end() const ;

    private:
        std::vector<std::string> names;
};

#endif

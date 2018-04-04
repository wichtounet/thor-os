//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef USER_FILE_HPP
#define USER_FILE_HPP

#include <types.hpp>
#include <expected.hpp>
#include <string.hpp>

#include "tlib/stat_info.hpp"
#include "tlib/statfs_info.hpp"
#include "tlib/config.hpp"
#include "tlib/directory_entry.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

std::expected<size_t> open(const char* file, size_t flags = 0);
int64_t mkdir(const char* file);
int64_t rm(const char* file);
std::expected<size_t> read(size_t fd, char* buffer, size_t max, size_t offset = 0);
std::expected<size_t> read(size_t fd, char* buffer, size_t max, size_t offset, size_t ms);
std::expected<size_t> write(size_t fd, const char* buffer, size_t max, size_t offset = 0);
std::expected<size_t> clear(size_t fd, size_t max, size_t offset = 0);
std::expected<size_t> truncate(size_t fd, size_t size);
std::expected<size_t> entries(size_t fd, char* buffer, size_t max);
void close(size_t fd);
std::expected<stat_info> stat(size_t fd);
std::expected<statfs_info> statfs(const char* file);
std::expected<size_t> mounts(char* buffer, size_t max);
std::expected<void> mount(size_t type, size_t dev_fd, size_t mp_fd);

std::string current_working_directory();
void set_current_working_directory(const std::string& directory);

struct directory_view;

/*!
 * \brief Represent a file
 */
struct file {
    /*!
     * \brief Open the file pointed by the given path
     * \param path The path to the file to open
     */
    file(const std::string& path);

    /*!
     * \brief Destroys the file and all resources acquired
     */
    ~file();

    /*!
     * \brief Indicates if the file is open or not
     * \return true if the file is open, false otherwise
     */
    bool open() const ;

    /*!
     * \brief Indicates if everything is in order
     * \return true if everything is good, false otherwise
     */
    bool good() const ;

    /*!
     * \brief Indicates if everything is in order
     * \return true if everything is good, false otherwise
     */
    operator bool();

    /*!
     * \brief Returns the error code, if any
     * \return the error code if any, 0 otherwise
     */
    size_t error() const ;

    /*!
     * \brief Returns the content of the file
     * \return The content of the file.
     *
     * The error code will be set if any error occurs
     */
    std::string read_file();

    /*!
     * \brief Returns an iterable structure for all files of the directory
     * \return an iterable structure for all files of the directory
     *
     * The error code will be set if any error occurs
     */
    directory_view entries();

private:
    std::string path;  ///< Path to the file
    size_t fd;         ///< The file descriptor
    size_t error_code; ///< The error code, if any
};

struct directory_iterator;

struct directory_view {
    directory_view(char* entries_buffer);

    ~directory_view();

    directory_iterator begin() const ;
    directory_iterator end() const ;

private:
    char* entries_buffer; ///< The buffer of entries

    friend struct directory_iterator;
};

struct directory_iterator {
    directory_iterator(const directory_view& view, bool end);

    const char* operator*() const ;

    directory_iterator& operator++();

    bool operator==(const directory_iterator& rhs) const ;
    bool operator!=(const directory_iterator& rhs) const ;

private:
    const directory_view& view;   ///< The originating view
    size_t position;              ///< The current position
    tlib::directory_entry* entry; ///< The current entry
    bool end;                     ///< Indicates the end of the entries
};

} // end of namespace tlib

#endif

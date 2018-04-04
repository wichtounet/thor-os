//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/file.hpp"

std::expected<size_t> tlib::open(const char* file, size_t flags){
    int64_t fd;
    asm volatile("mov rax, 0x300; mov rbx, %[path]; mov rcx, %[flags]; int 50; mov %[fd], rax"
        : [fd] "=m" (fd)
        : [path] "g" (reinterpret_cast<size_t>(file)), [flags] "g" (flags)
        : "rax", "rbx", "rcx");

    if(fd < 0){
        return std::make_expected_from_error<size_t, size_t>(-fd);
    } else {
        return std::make_expected<size_t>(fd);
    }
}

int64_t tlib::mkdir(const char* file){
    int64_t result;
    asm volatile("mov rax, 0x306; mov rbx, %[path]; int 50; mov %[result], rax"
        : [result] "=m" (result)
        : [path] "g" (reinterpret_cast<size_t>(file))
        : "rax", "rbx");
    return result;
}

int64_t tlib::rm(const char* file){
    int64_t result;
    asm volatile("mov rax, 0x307; mov rbx, %[path]; int 50; mov %[result], rax"
        : [result] "=m" (result)
        : [path] "g" (reinterpret_cast<size_t>(file))
        : "rax", "rbx");
    return result;
}

void tlib::close(size_t fd){
    asm volatile("mov rax, 0x302; mov rbx, %[fd]; int 50;"
        : /* No outputs */
        : [fd] "g" (fd)
        : "rax", "rbx");
}

std::expected<tlib::stat_info> tlib::stat(size_t fd){
    tlib::stat_info info;

    int64_t code;
    asm volatile("mov rax, 0x301; mov rbx, %[fd]; mov rcx, %[buffer]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [buffer] "g" (reinterpret_cast<size_t>(&info))
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<tlib::stat_info, size_t>(-code);
    } else {
        return std::make_expected<tlib::stat_info>(info);
    }
}

std::expected<tlib::statfs_info> tlib::statfs(const char* file){
    tlib::statfs_info info;

    int64_t code;
    asm volatile("mov rax, 0x310; mov rbx, %[path]; mov rcx, %[buffer]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [path] "g" (reinterpret_cast<size_t>(file)), [buffer] "g" (reinterpret_cast<size_t>(&info))
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<tlib::statfs_info, size_t>(-code);
    } else {
        return std::make_expected<tlib::statfs_info>(info);
    }
}

std::expected<size_t> tlib::read(size_t fd, char* buffer, size_t max, size_t offset){
    int64_t code;
    asm volatile("mov rax, 0x303; mov rbx, %[fd]; mov rcx, %[buffer]; mov rdx, %[max]; mov rsi, %[offset]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [buffer] "g" (reinterpret_cast<size_t>(buffer)), [max] "g" (max), [offset] "g" (offset)
        : "rax", "rbx", "rcx", "rdx", "rsi");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}
std::expected<size_t> tlib::read(size_t fd, char* buffer, size_t max, size_t offset, size_t ms){
    int64_t code;
    asm volatile("mov rax, 0x315; mov rbx, %[fd]; mov rcx, %[buffer]; mov rdx, %[max]; mov rsi, %[offset]; mov rdi, %[ms]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [buffer] "g" (reinterpret_cast<size_t>(buffer)), [max] "g" (max), [offset] "g" (offset), [ms] "g" (ms)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> tlib::write(size_t fd, const char* buffer, size_t max, size_t offset){
    int64_t code;
    asm volatile("mov rax, 0x311; mov rbx, %[fd]; mov rcx, %[buffer]; mov rdx, %[max]; mov rsi, %[offset]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [buffer] "g" (reinterpret_cast<size_t>(buffer)), [max] "g" (max), [offset] "g" (offset)
        : "rax", "rbx", "rcx", "rdx", "rsi");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> tlib::clear(size_t fd, size_t max, size_t offset){
    int64_t code;
    asm volatile("mov rax, 0x313; mov rbx, %[fd]; mov rcx, %[max]; mov rdx, %[offset]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [max] "g" (max), [offset] "g" (offset)
        : "rax", "rbx", "rcx", "rdx");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> tlib::truncate(size_t fd, size_t size){
    int64_t code;
    asm volatile("mov rax, 0x312; mov rbx, %[fd]; mov rcx, %[size]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [size] "g" (size)
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> tlib::entries(size_t fd, char* buffer, size_t max){
    int64_t code;
    asm volatile("mov rax, 0x308; mov rbx, %[fd]; mov rcx, %[buffer]; mov rdx, %[max]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [buffer] "g" (reinterpret_cast<size_t>(buffer)), [max] "g" (max)
        : "rax", "rbx", "rcx", "rdx");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> tlib::mounts(char* buffer, size_t max){
    int64_t code;
    asm volatile("mov rax, 0x309; mov rbx, %[buffer]; mov rcx, %[max]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [buffer] "g" (reinterpret_cast<size_t>(buffer)), [max] "g" (max)
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<void> tlib::mount(size_t type, size_t dev_fd, size_t mp_fd){
    int64_t code;
    asm volatile("mov rax, 0x314; mov rbx, %[type]; mov rcx, %[mp]; mov rdx, %[dev]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [type] "g" (type), [dev] "g" (dev_fd), [mp] "g" (mp_fd)
        : "rax", "rbx", "rcx", "rdx");

    if(code < 0){
        return std::make_expected_from_error<void, size_t>(-code);
    } else {
        return std::make_expected();
    }
}

std::string tlib::current_working_directory(){
    char buffer[128];
    buffer[0] = '\0';

    asm volatile("mov rax, 0x304; mov rbx, %[buffer]; int 50;"
        : /* No outputs */
        : [buffer] "g" (reinterpret_cast<size_t>(buffer))
        : "rax", "rbx");

    return {buffer};
}

void tlib::set_current_working_directory(const std::string& directory){
    asm volatile("mov rax, 0x305; mov rbx, %[buffer]; int 50;"
        : /* No outputs */
        : [buffer] "g" (reinterpret_cast<size_t>(directory.c_str()))
        : "rax", "rbx");
}

tlib::file::file(const std::string& path) : path(path), fd(0), error_code(0) {
    auto open_status = tlib::open(path.c_str());

    if(open_status.valid()){
        fd = *open_status;
    } else {
        error_code = open_status.error();
    }
}

tlib::file::~file(){
    if(fd){
        tlib::close(fd);
    }
}

bool tlib::file::open() const {
    return fd > 0;
}

bool tlib::file::good() const {
    return error_code == 0;
}

tlib::file::operator bool(){
    return good();
}

size_t tlib::file::error() const {
    return error_code;
}

std::string tlib::file::read_file(){
    if(!good() || !open()){
        return "";
    }

    std::string buffer;

    auto info = tlib::stat(fd);

    if(info.valid()){
        auto size = info->size;
        size_t read = 0;

        char raw_buffer[1024 + 1];

        while(read < size){
            auto content_result = tlib::read(fd, raw_buffer, 1024, read);

            if(content_result.valid()){
                auto off = *content_result;

                raw_buffer[off] = '\0';
                buffer += raw_buffer;
                read += off;
            } else { error_code = content_result.error();
                return "";
            }
        }
    } else {
        error_code = info.error();
        return "";
    }

    return buffer;
}

tlib::directory_view tlib::file::entries(){
    if(!good() || !open()){
        return {nullptr};
    }

    auto entries_buffer = new char[4096];

    auto entries_result = tlib::entries(fd, entries_buffer, 4096);

    if(!entries_result){
        delete[] entries_buffer;
        error_code = entries_result.error();

        return {nullptr};
    }

    return {entries_buffer};
}

tlib::directory_iterator::directory_iterator(const directory_view& view, bool end) : view(view), position(0), end(end) {
    if(!end){
        entry = reinterpret_cast<tlib::directory_entry*>(view.entries_buffer + position);
    }
}

const char* tlib::directory_iterator::operator*() const {
    return &entry->name;
}

tlib::directory_iterator& tlib::directory_iterator::operator++(){
    if(end){
        return *this;
    }

    if(!entry->offset_next){
        entry = nullptr;
        end = true;
        return *this;
    }

    position += entry->offset_next;
    entry = reinterpret_cast<tlib::directory_entry*>(view.entries_buffer + position);

    return *this;
}

bool tlib::directory_iterator::operator==(const directory_iterator& rhs) const {
    if(end){
        return rhs.end;
    } else if(rhs.end){
        return false;
    } else {
        return position == rhs.position;
    }
}

bool tlib::directory_iterator::operator!=(const directory_iterator& rhs) const {
    return !(*this == rhs);
}

tlib::directory_view::directory_view(char* entries_buffer) : entries_buffer(entries_buffer) {}

tlib::directory_view::~directory_view(){
    if(entries_buffer){
        delete[] entries_buffer;
    }
}

tlib::directory_iterator tlib::directory_view::begin() const {
    if(entries_buffer){
        return {*this, false};
    } else {
        return {*this, true};
    }
}

tlib::directory_iterator tlib::directory_view::end() const {
    return {*this, true};
}

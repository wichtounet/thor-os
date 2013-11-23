#include "string.hpp"
#include "utils.hpp"


string::string(const char* s){
    _size = str_len(s);
    _capacity = _size + 1;
    _data = new char[_capacity];
    memcopy(_data, s, _capacity);
}

string::string(size_t capacity){
    _size = 0;
    _capacity = capacity;
    _data = new char[_capacity];
}

string::string(const string& rhs){
    _size = rhs._size;
    _capacity = rhs._capacity;
    _data = new char[_capacity];
    memcopy(_data, rhs._data, _capacity);
}

string& string::operator=(const string& rhs){
    if(this != &rhs){
        _size = rhs._size;
        _capacity = rhs._capacity;
        _data = new char[_capacity];
        memcopy(_data, rhs._data, _capacity);
    }

    return *this;
}

string::string(string&& rhs){
    _size = rhs._size;
    _capacity = rhs._capacity;
    _data = rhs._data;

    rhs._size = 0;
    rhs._capacity = 0;
    rhs._data = nullptr;
}

string& string::operator=(string&& rhs){
    _size = rhs._size;
    _capacity = rhs._capacity;
    _data = rhs._data;

    rhs._size = 0;
    rhs._capacity = 0;
    rhs._data = nullptr;

    return *this;
}

string::~string(){
    delete[] _data;
}

size_t string::size() const {
    return _size;
}

const char* string::c_str() const {
    return _data;
}

string::iterator string::begin(){
    return _data;
}

string::iterator string::end(){
    return _data + _size;
}

string::const_iterator string::begin() const {
    return _data;
}

string::const_iterator string::end() const {
    return _data + _size;
}

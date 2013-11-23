#include "string.hpp"
#include "utils.hpp"

string::string(){
    _size = 0;
    _capacity = 0;
    _data = nullptr;
}

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
    *this = rhs;
}

string& string::operator=(const string& rhs){
    if(this != &rhs){
        if(_capacity < rhs._capacity){
            if(_data){
                delete[] _data;
            }

            _capacity = rhs._capacity;
            _data = new char[_capacity];
        }

        _size = rhs._size;
        memcopy(_data, rhs._data, _size + 1);
    }

    return *this;
}

string::string(string&& rhs){
    *this = rhs;
}

string& string::operator=(string&& rhs){
    if(_data){
        delete[] _data;
    }

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

bool string::empty() const {
    return _size;
}

const char* string::c_str() const {
    return _data;
}

string string::operator+(char c) const {
    string copy = *this;

    copy += c;

    return move(copy);
}

string& string::operator+=(char c){
    if(!_data || _capacity <= _size + 1){
        if(_data){
            delete[] _data;
        }

        _capacity = _size + 3;
        _data = new char[_capacity];
    }

    _data[_size] = c;
    _data[++_size] = '\0';

    return *this;
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

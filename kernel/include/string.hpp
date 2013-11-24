#ifndef STRING_H
#define STRING_H

#include "types.hpp"

struct string {
public:
    typedef char*             iterator;
    typedef const char*       const_iterator;

private:
    char* _data;
    size_t _size;
    size_t _capacity;

public:
    //Constructors

    string();
    string(const char* s);
    explicit string(size_t capacity);

    //Copy constructors

    string(const string& rhs);
    string& operator=(const string& rhs);

    //Move constructors

    string(string&& rhs);
    string& operator=(string&& rhs);

    //Destructors

    ~string();

    //Modifiers

    void clear();

    //Accessors

    size_t size() const;
    size_t capacity() const;
    bool empty() const;

    const char* c_str() const;

    //Operators

    string operator+(char c) const;
    string& operator+=(char c);

    //Iterators

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;
};

#endif

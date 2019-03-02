#pragma once


#include <iostream>

namespace cf {

class Buffer {
public:
    Buffer(const void* buffer, int size);
    Buffer(int size, char c = '\0');
    Buffer(const std::string& s);
    Buffer(const char * str);
    Buffer();
    Buffer(const Buffer& o);
    ~Buffer();

    int length() const { return len; }
    const char* buffer() const { return data; }
    const char * offset(int offset) const { return data + offset; }
    bool isEmpty();

    void assign(const void* data, int len);
    void assign(int size, char c = '\0');
    void clear() { assign(0); }

    void append(const void* data, int size);
    void append(const Buffer& o);
    void erase_before(int offset);

    char& at(int offset);
    char& operator[](int offset);
    char operator[] (int offset) const;
    operator char*() { return data; }
    operator const char*() { return data; }
    operator unsigned char*() { return (unsigned char*)data; }
    operator void*() { return data; }
    Buffer& operator=(const Buffer& o);
    Buffer& operator=(const Buffer* o);
    unsigned char* operator+(int offset) { return (unsigned char*)data + offset; }

    void copy(const Buffer& o);
    void move(const Buffer& o);

    int     mArgN;
    long    mArgL;
    void*   mArgP;

private:

    int     len;
    char*   data;
};

} // namespace cf 


std::ostream& operator<<(std::ostream& o, const cf::Buffer& ub);


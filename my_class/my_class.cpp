/*! \file my_class.cpp
 * \brief MyClass class implementation.
 */

#include <iostream>
#include <utility>

#include "my_class.h"

using namespace std;

MyClass::MyClass(const char* data) : _data(nullptr)
{
    if (data)
    {
        std::size_t len = std::strlen(data) + 1;
        _data = new char[len];
        std::memcpy(_data, data, len);
    }
}

MyClass::~MyClass()
{
    delete[] _data;
};

MyClass::MyClass(const MyClass& other) : MyClass(other._data)
{
}

MyClass::MyClass(MyClass&& other) noexcept : _data(std::exchange(other._data, nullptr))
{
}

MyClass& MyClass::operator=(const MyClass& other)
{
    return *this = MyClass(other._data);
}

MyClass& MyClass::operator=(MyClass&& other) noexcept
{
    std::swap(_data, other._data);
    return *this;
}

bool MyClass::is_data_null() const
{
    return _data == nullptr;
}

std::string MyClass::get() const
{
    return string(_data);
}

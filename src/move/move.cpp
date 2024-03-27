/*! \file move.cpp
 * \brief Move class implementation.
 */

#include <iostream>
#include <utility>

#include "move.h"

using namespace std;

Move::Move(const char* str_prt, const std::string& str) : _str_ptr(nullptr), _str(str)
{
    cout << "Move::Move(const char* str_prt, const std::string& str)" << endl;
    if (str_prt)
    {
        auto len = std::strlen(str_prt) + 1;
        _str_ptr = new char[len];
        std::memcpy(_str_ptr, str_prt, len);
    }
}

Move::Move(const Move& other) : _str_ptr(other._str_ptr), _str(other._str)
{
    cout << "Move::Move(const Move& other)" << endl;
}

Move& Move::operator=(const Move& other)
{
    cout << "Move& Move::operator=(const Move& other)" << endl;
    if (this != &other)
    {
        delete[] _str_ptr;
        size_t len = strlen(other._str_ptr) + 1;
        _str_ptr = new char[len];
        strcpy(_str_ptr, other._str_ptr);
        _str = other._str;
    }

    return *this;
}

Move::Move(Move&& other) noexcept : _str_ptr(std::exchange(other._str_ptr, nullptr)), _str(std::move(other._str))
{
    cout << "Move::Move(Move&& other)" << endl;
}

Move& Move::operator=(Move&& other) noexcept
{
    cout << "Move& Move::operator=(Move&& other)" << endl;
    std::swap(_str_ptr, other._str_ptr);
    _str = std::move(other._str);
    return *this;
}

void Move::print()
{
    cout << "str: " << _str << endl;
    cout << "str_prt: " << _str_ptr << endl;
}

Move Move::operator+(const Move& other)
{
    cout << "Move &Move::operator+(const Move &other)" << endl;
    char* str_ptr = new char[strlen(this->_str_ptr) + strlen(other._str_ptr) + 1];
    strcpy(str_ptr, this->_str_ptr);
    strcpy(str_ptr + strlen(this->_str_ptr), other._str_ptr);
    Move res(str_ptr, this->_str + other._str);
    delete[] str_ptr;
    return res;
}

std::string Move::get_str() const
{
    return this->_str;
}

const char* Move::get_str_prt() const
{
    return this->_str_ptr;
}

Move::~Move()
{
    if (_str_ptr)
        delete[] _str_ptr;
}

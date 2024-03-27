/*! \file my_class.h
 * \brief MyClass class interface.
 *
 * Class description.
 *
 */

#include <cstring>

#pragma once

/*! \class MyClass
 * \brief Some briefing
 */

class MyClass
{
public:
    //! \brief constructor.
    MyClass(const char* data = "");

    //! \brief copy constructor.
    MyClass(const MyClass& other);

    //! \brief move constructor.
    MyClass(MyClass&& other) noexcept;

    //! \brief copy assigment
    MyClass& operator=(const MyClass& other);

    //! \brief move assigment
    MyClass& operator=(MyClass&& other) noexcept;

    //! \brief destructor.
    virtual ~MyClass();

    //! \brief get data
    virtual std::string get() const;

    //! \brief check data field is nullptr
    virtual bool is_data_null() const;

private:
    //! \brief raw pointer
    char* _data;
};

/*! \file move.h
 * \brief Move class interface.
 *
 * Class description.
 *
 */


#pragma once

#include <string>
#include <cstring>

/*! \class Move
 * \brief Some briefing
 */
class Move
{
public:

    //! \brief constructor.
    explicit Move(const char* str_prt = "Hi ptr", const std::string& str = std::string("Hi mark"));

    //! \brief copy constructor.
    Move(const Move& other);

    //! \brief assigment operator.
    Move& operator=(const Move& other);

    //! \brief plus operator.
    Move operator+(const Move& other);

    //! \brief move constructor.
    Move(Move&& other) noexcept;

    //! \brief move assigment operator.
    Move& operator=(Move&& other) noexcept;

    //! \brief default destructor.
    ~Move();

    void print();

    std::string get_str() const;

    const char*  get_str_prt() const;

private:
    char* _str_ptr;
    std::string _str;
};

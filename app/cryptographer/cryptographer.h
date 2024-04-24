/*! \file cryptographer.h
 * \brief Cryptographer class interface.
 *
 * Class description.
 *
 */


#pragma once

#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include <cryptopp/cryptlib.h>

/*! \class Cryptographer
 * \brief Some briefing
 */
class Cryptographer {
    Cryptographer(const Cryptographer&) = delete;
    Cryptographer(Cryptographer&&) = delete;
    Cryptographer operator=(const Cryptographer&) = delete;
    Cryptographer operator=(Cryptographer&&) = delete;
public:

    //! \brief default constructor.
    Cryptographer();

    //! \brief default destructor.
    ~Cryptographer() = default;

    void encode(const std::string& str);
    void decode();
private:

    //! List of private variables.
};

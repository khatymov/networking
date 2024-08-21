/*! \file cryptographer.h
 * \brief Cryptographer class interface.
 *
 * Class description.
 *
 */


#pragma once

#include "packet.h"

#include <iostream>
#include <cryptopp/aes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <string>

using namespace CryptoPP;

class Cryptographer {
public:

    void encrypt(const Packet& source, CryptoPacket& cipher);

    void decrypt(const CryptoPacket& cipher, Packet& plainPacket);

    // Function to generate a random key
    static void GenerateAndSaveKey(const std::string& filename);

    // set a key from env var
    bool setKey(const std::string& envKeyName);

private:
    std::string _key = "5E462EA6BD40B083F5F2C4B810A07230";
};
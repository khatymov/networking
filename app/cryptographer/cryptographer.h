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
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>
#include <cryptopp/files.h>
#include <string>

using namespace CryptoPP;

class CryptographerImpl {
public:
    // Why should we use new in ArraySource
    //    https://cryptopp.com/wiki/Pipelining#Ownership
    void encrypt(const Packet& source, CryptoPacket& cipher) {

        CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
        memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

        cipher.header.type = source.header.type;
        cipher.header.length = source.header.length + AES::BLOCKSIZE;

        try {
            CBC_Mode<AES>::Encryption enc;
            enc.SetKeyWithIV((CryptoPP::byte*)_key.data(), _key.size(), iv, sizeof(iv));
            // Make room for padding
            ArraySink cs(&cipher.payload[0], source.header.length + AES::BLOCKSIZE);
            ArraySource(reinterpret_cast<const CryptoPP::byte*>(source.payload), source.header.length, true, new StreamTransformationFilter(enc, new Redirector(cs)));
            // Set cipher text length now that its known
            cipher.header.length = cs.TotalPutLength();
        } catch (const CryptoPP::Exception& e) {
            std::cerr << e.what() << std::endl;
            exit(1);
        }
    }

    void decrypt(const CryptoPacket& cipher, Packet& plainPacket) {
        CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
        memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

        plainPacket.header.type = cipher.header.type;
        //TODO: rethink approach
        std::vector<CryptoPP::byte> recover;
        recover.resize(cipher.header.length);

        try {
            CBC_Mode<AES>::Decryption dec;
            dec.SetKeyWithIV((CryptoPP::byte*)_key.data(), _key.size(), iv, sizeof(iv));
            ArraySink rs(&recover[0], cipher.header.length);
            ArraySource(cipher.payload.data(), cipher.header.length, true, new StreamTransformationFilter(dec, new Redirector(rs)));
            recover.resize(rs.TotalPutLength());
            plainPacket.header.length = rs.TotalPutLength();
            std::copy(recover.begin(), recover.end(), plainPacket.payload);
        } catch (const CryptoPP::Exception& e) {
            std::cerr << e.what() << std::endl;
            exit(1);
        }
    }

    // Function to generate a random key
    static void GenerateAndSaveKey(const std::string& filename) {
        CryptoPP::AutoSeededRandomPool rnd;
        CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);

        // Generate a random key
        rnd.GenerateBlock(key, key.size());

        // Save the key to a file
        CryptoPP::HexEncoder encoder(new CryptoPP::FileSink(filename.c_str()));
        encoder.Put(key, key.size());
        encoder.MessageEnd();

        std::cout << "Key saved to " << filename << std::endl;
    }

    // set a key from env var
    bool setKey(const std::string& envKeyName) {
        const char* envKey = std::getenv(envKeyName.c_str());
        if (!envKey) {
            std::cerr << "Environment variable MY_APP_KEY is not set!" << std::endl;
            return false;
        } else {
            _key = envKey;
        }

        return true;
    };

private:
    std::string _key;// = "5E462EA6BD40B083F5F2C4B810A07230";
};
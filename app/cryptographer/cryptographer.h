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
        cipher.realDatalength = source.header.length;
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

    Packet decrypt(const CryptoPacket& cipher) {
        Packet source;

        CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
        memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

        source.header.type = cipher.header.type;
        source.header.length = cipher.realDatalength;
//        source.header.length = cipher.header.length;

        try {
            CBC_Mode<AES>::Decryption dec;
            dec.SetKeyWithIV((CryptoPP::byte*)_key.data(), _key.size(), iv, sizeof(iv));
            ArraySink rs(reinterpret_cast<CryptoPP::byte*>(source.payload), cipher.header.length);
            ArraySource(cipher.payload.data(), cipher.header.length, true, new StreamTransformationFilter(dec, new Redirector(rs)));
        } catch (const CryptoPP::Exception& e) {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        return source;
    }

    // Function to generate a random key
    void GenerateAndSaveKey(const std::string& filename) {
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
    bool setKey() {
        const char* envKey = std::getenv("myKey");
        if (!envKey) {
            std::cerr << "Environment variable MY_APP_KEY is not set!" << std::endl;
            return false;
        } else {
            _key = envKey;
        }

        return true;
    };

private:
    std::string _key = "5E462EA6BD40B083F5F2C4B810A07230";
};
/*! \file cryptographer.cpp
 * \brief Cryptographer class implementation.
 */

#include "cryptographer.h"

#include <string>

using namespace std;
using namespace CryptoPP;

//To delete
#include <iostream>
#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>

AutoSeededRandomPool prng;

Cryptographer::Cryptographer() {}

void Cryptographer::encode(const string& str) {
    const char* envKey = std::getenv("myKey");
    if (!envKey) {
        std::cerr << "Environment variable MY_APP_KEY is not set!" << std::endl;
        return;
    }

    string key(envKey); // ("gIoi/23NZ06Cm8pelo74mw=="); //(AES::DEFAULT_KEYLENGTH);

//    prng.GenerateBlock(key, key.size());

//    std::string encoded;
//    Base64Encoder encoder;
//
//    encoder.Attach(new StringSink(encoded));
//    encoder.Put(key, key.size());
//    encoder.MessageEnd();
//
//    std::cout << "Encoded Key: " << encoded << std::endl;

    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
//    memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
    memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

    char plaintext[] = "Hello World!";  // Data to encrypt
    std::string ciphertext;
    try {
        CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor;
        encryptor.SetKeyWithIV((CryptoPP::byte*)key.data(), sizeof(key), iv);

        // The StreamTransformationFilter adds padding as required. AES uses PKCS #7 padding by default.
        CryptoPP::StringSource ss(plaintext, true,
                                  new CryptoPP::StreamTransformationFilter(encryptor,
                                                                           new CryptoPP::StringSink(ciphertext)
                                                                               ) // StreamTransformationFilter
        ); // StringSource
    }
    catch(const CryptoPP::Exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    std::string encoded;
    CryptoPP::StringSource ss2(ciphertext, true,
                               new CryptoPP::HexEncoder(
                                   new CryptoPP::StringSink(encoded)
                                       ) // HexEncoder
    ); // StringSource

    std::cout << "Ciphertext (Hex): " << encoded << std::endl;

    CryptoPP::byte iv2[CryptoPP::AES::BLOCKSIZE];
    //    memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
    memset(iv2, 0x00, CryptoPP::AES::BLOCKSIZE);


//    key[key.size() - 1] = '=';
    std::string decryptedtext;
    try {
        CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;
        decryptor.SetKeyWithIV((CryptoPP::byte*)key.data(), sizeof(key), iv2);

        // The StreamTransformationFilter removes
        // padding as required.
        CryptoPP::StringSource s(ciphertext, true,
                                 new CryptoPP::StreamTransformationFilter(decryptor,
                                                                          new CryptoPP::StringSink(decryptedtext)
                                                                              ) // StreamTransformationFilter
        ); // StringSource
    }
    catch(const CryptoPP::Exception& e) {
        std::cerr << e.what() << std::endl;
//        return 1;
    }

    std::cout << "Decrypted text: " << decryptedtext << std::endl;
}

void Cryptographer::decode() {

}

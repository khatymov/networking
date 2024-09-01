/*! \file cryptographer.cpp
 * \brief Cryptographer class implementation.
 */

#include "cryptographer.h"

void Cryptographer::encrypt(const Packet& source, CryptoPacket& cipher) {
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
    memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

    cipher.header.type = source.header.type;
    cipher.header.length = source.header.length + AES::BLOCKSIZE;

    try {
        CBC_Mode<AES>::Encryption enc;
        enc.SetKeyWithIV((CryptoPP::byte*)_key.data(), _key.size(), iv, sizeof(iv));
        // Make room for padding
        ArraySink cs(&cipher.payload[0],
                     source.header.length + AES::BLOCKSIZE);
        // Why should we use new in ArraySource
        //    https://cryptopp.com/wiki/Pipelining#Ownership
        ArraySource(reinterpret_cast<const CryptoPP::byte*>(source.payload),
                    source.header.length,
                    true,
                    new StreamTransformationFilter(enc, new Redirector(cs)));
        // Set cipher text length now that its known
        cipher.header.length = cs.TotalPutLength();
    } catch (const CryptoPP::Exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

void Cryptographer::decrypt(const CryptoPacket& cipher, Packet& plainPacket) {
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
    memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

    plainPacket.header.type = cipher.header.type;
    //TODO: rethink approach
    //I understand that copy is a bad idea
    std::vector<CryptoPP::byte> recover;
    recover.resize(cipher.header.length);

    try {
        CBC_Mode<AES>::Decryption dec;
        dec.SetKeyWithIV((CryptoPP::byte*)_key.data(), _key.size(), iv, sizeof(iv));
        ArraySink rs(&recover[0], cipher.header.length);
        // Why should we use new in ArraySource
        //    https://cryptopp.com/wiki/Pipelining#Ownership
        ArraySource(cipher.payload.data(),
                    cipher.header.length,
                    true,
                    new StreamTransformationFilter(dec, new Redirector(rs)));
        recover.resize(rs.TotalPutLength());
        plainPacket.header.length = rs.TotalPutLength();
        std::copy(recover.begin(), recover.end(), plainPacket.payload);
    } catch (const CryptoPP::Exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

void Cryptographer::GenerateAndSaveKey(const std::string& filename) {
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

bool Cryptographer::setKey(const std::string& envKeyName) {
    const char* envKey = std::getenv(envKeyName.c_str());
    if (!envKey) {
        std::cerr << "Environment variable MY_APP_KEY is not set!" << std::endl;
        return false;
    } else {
        _key = envKey;
    }

    return true;
}

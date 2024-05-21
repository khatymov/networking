#include "gtest/gtest.h"

#include "encryption_test.h"

#include "file_handler.h"

using namespace std;
using namespace testing;
using namespace CryptoPP;

//To delete
#include <iostream>
#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>
#include <cryptopp/files.h>

#include <span>

template <typename R>
class CryptographerInterface {
public:
    virtual void encrypt(std::span<R> source, std::span<R> cipher) = 0;
    virtual ~CryptographerInterface() = default;
};

template <typename T, typename K>
class ICryptographer {
public:
    virtual void setKey() = 0;
    virtual void encrypt(T& plainText, K& ciphertext, size_t& outLen) = 0;
    virtual void decrypt(K& ciphertext, T& plainText) = 0;
    virtual ~ICryptographer() = default;
};


class ICryptographerInterface {
public:
    virtual void encrypt(const Packet& source, CryptoPacket& cipher) = 0;
    virtual void decrypt(const CryptoPacket& cipher, Packet& source) = 0;
    virtual ~ICryptographerInterface() = default;
};

class CryptoImpl: public ICryptographerInterface {
public:


    void encrypt(const Packet& source, CryptoPacket& cipher) override {

        CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
        memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

        HexEncoder encoder(new FileSink(cout));

        try {
            CBC_Mode<AES>::Encryption enc;
            enc.SetKeyWithIV((CryptoPP::byte*)_key.data(), _key.size(), iv, sizeof(iv));

            // Make room for padding
//            cipher.resize(plain.size()+AES::BLOCKSIZE);
            ArraySink cs(&cipher.payload[0], source.header.length + AES::BLOCKSIZE);

            const char* tmp = source.payload;

            ArraySource(reinterpret_cast<const CryptoPP::byte*>(source.payload), source.header.length, true, new StreamTransformationFilter(enc, new Redirector(cs)));

            // Set cipher text length now that its known
            cipher.header.length = cs.TotalPutLength();

            cout << "Cipher text: ";
            encoder.Put(&cipher.payload[0], cipher.header.length);
            encoder.MessageEnd();
            cout << endl;
        } catch (const CryptoPP::Exception& e) {
            std::cerr << e.what() << std::endl;
            exit(1);
        }
    }

    void decrypt(const CryptoPacket& cipher, Packet& source) override {

        HexEncoder encoder(new FileSink(cout));

        CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
        memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

        CBC_Mode<AES>::Decryption dec;
        dec.SetKeyWithIV((CryptoPP::byte*)_key.data(), _key.size(), iv, sizeof(iv));

        const auto sz = cipher.header.length;
//        ArraySink rs(&source.payload, sz);
        ArraySink rs(reinterpret_cast<CryptoPP::byte*>(source.payload), sz);

        ArraySource(cipher.payload.data(), cipher.header.length, true,
                    new StreamTransformationFilter(dec, new Redirector(rs)));


        source.header.length = rs.TotalPutLength();
        cout << "Recovered size: " << rs.TotalPutLength()  <<" \ntext: " << source.payload;
//        encoder.Put(&source.payload[0], sz);
//        encoder.MessageEnd();
        cout << endl;
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
    string _key = "5E462EA6BD40B083F5F2C4B810A07230";
};

TEST(test_encryption, test_char_array) {


    std::string keyFile = "key.hex";

    // encrypt plain packet into crypto packet
    CryptoPacket cryptoPacket;
    {
        FileHandler fileReader;
        fileReader.open("../../data/Somedata.txt", "rb");
        Packet packetClient;
        fileReader.read(packetClient);
        CryptoImpl crypto_impl_client;


        // Generate and save the key
//        crypto_impl_client.GenerateAndSaveKey(keyFile);
        crypto_impl_client.encrypt(packetClient, cryptoPacket);
    }


    // decrypt crypto packet into plain packet

    {
        Packet packetServer;
        packetServer.payload[0] = 'H';
        FileHandler fileWriter;
        fileWriter.open("recovered.txt", "w");
        CryptoImpl crypto_impl_server;
        crypto_impl_server.decrypt(cryptoPacket, packetServer);
//        cout << packetServer.payload << endl;
        fileWriter.write(packetServer);
        fileWriter.close();
    }




    // зашифровать данные в пакете так, чтобы класс можно было переиспользовать для простого шифрования строки
//    CryptographerImlp<char []> cryptographerImlp;

//    CryptographerImlp<CryptoPP::byte [] , char []> decryptor;
//    decryptor.setKey();
//    Packet res;
//    decryptor.decrypt(cipherText, res.payload);
//    cout << res.payload << endl;
}

template <typename T, typename K>
class CryptographerImlp: public ICryptographer<T, K> {
public:
    // set a key from env var
    void setKey() override {
        const char* envKey = std::getenv("myKey");
        if (!envKey) {
            std::cerr << "Environment variable MY_APP_KEY is not set!" << std::endl;
            return;
        }
        _key = envKey;
    };

    //
    void encrypt(T& plainText, K& cipherText, size_t& outLen) override {
        cout << "CryptoPP::AES::BLOCKSIZE: " << CryptoPP::AES::BLOCKSIZE << "\n";
        CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
        //    memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
        memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

        //        char plaintext[] = "Hello World!";  // Data to encrypt
        //        std::string ciphertext;

        try {
            //CBC Mode is cipher block chaining
            CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor;
            encryptor.SetKeyWithIV((CryptoPP::byte*)_key.data(), _key.size(), iv);

            CryptoPP::ArraySink cs(cipherText, outLen);
            CryptoPP::StreamTransformationFilter st;
            // The StreamTransformationFilter adds padding as required. AES uses PKCS #7 padding by default.
            CryptoPP::StringSource ss(plainText, true,
                                      new CryptoPP::StreamTransformationFilter(encryptor, new CryptoPP::ArraySink(cipherText, DATA_SIZE)
                                                                                   ) // StreamTransformationFilter
            ); // StringSource
                                                                                      //            outLen = cs.TotalPutLength();
                                                                                      //            ciphertext.size();
                                                                                      //            std::cout << ciphertext.size() << std::endl;
        }
        catch(const CryptoPP::Exception& e) {
            std::cerr << e.what() << std::endl;
            exit(1);
        }
    }

    void decrypt(T& ciphertext, K& plainText) override {
        CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
        //    memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
        memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);


        //    key[key.size() - 1] = '=';
        //        std::string decryptedtext;
        try {
            CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;
            decryptor.SetKeyWithIV((CryptoPP::byte*)_key.data(), _key.size(), iv);

            // The StreamTransformationFilter removes
            // padding as required.
            CryptoPP::ArraySource s(ciphertext, true,
                                    new CryptoPP::StreamTransformationFilter(decryptor,
                                                                             new CryptoPP::ArraySink(plainText, DATA_SIZE)
                                                                                 ) // StreamTransformationFilter
            ); // StringSource
        }
        catch(const CryptoPP::Exception& e) {
            std::cerr << e.what() << std::endl;
            //        return 1;
        }
    }
    ~CryptographerImlp() override {

    }

private:
    string _key;
    //    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE] = {8,7,6,5, 8,7,6,5, 8,7,6,5, 8,7,6,5};
};
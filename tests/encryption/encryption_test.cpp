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

template <typename T, typename K>
class ICryptographer {
public:
    virtual void setKey() = 0;
    virtual void encrypt(T& plainText, K& ciphertext, size_t& outLen) = 0;
    virtual void decrypt(T& ciphertext, K& plainText) = 0;
    virtual ~ICryptographer() {};
};

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

//            CryptoPP::ArraySink cs(cipherText, outLen);
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

TEST(test_encryption, test_char_array)
{
    FileHandler fileReader;
    fileReader.open("../../data/Somedata.txt", "rb");
    Packet packet;
    fileReader.read(packet);
    // зашифровать данные в пакете так, чтобы класс можно было переиспользовать для простого шифрования строки
//    CryptographerImlp<char []> cryptographerImlp;
    CryptographerImlp<char [], CryptoPP::byte []> encrypter;
//    CryptographerImlp<char *, CryptoPP::byte *> encrypter;

    CryptoPP::byte cipherText[DATA_SIZE];
    encrypter.setKey();
//    string cipherText;
    size_t outSize = packet.header.length + 5;
    encrypter.encrypt(packet.payload, cipherText, outSize);

//    FileHandler fileWriter;
//    fileWriter.open("../../data/Decrypted2.txt", "w");

    CryptographerImlp<CryptoPP::byte [] , char []> decryptor;
    decryptor.setKey();
    Packet res;
    decryptor.decrypt(cipherText, res.payload);
    cout << res.payload << endl;
}

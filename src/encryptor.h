//
// Created by Renat_Khatymov on 9/1/2024.
//

#ifndef NETWORKING_ENCRYPTOR_H
#define NETWORKING_ENCRYPTOR_H

#include "data_processor_interface.h"
#include "my_packet.h"
#include "pch.h"

namespace network {

using namespace CryptoPP;

const std::string EncryptorKey("Encryptor");

template <typename DataType>
class Encryptor: public DataProcessor<Encryptor<DataType>, DataType> {
    Encryptor(const Encryptor&) = delete;
    Encryptor(Encryptor&&) = delete;
    Encryptor& operator=(const Encryptor&) = delete;
    Encryptor& operator=(Encryptor&&) = delete;
public:
    Encryptor(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
              std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue);

    // plain to cipher without overhead
    void processDataImpl();


protected:
    void setKey();

    std::unique_ptr<DataType> cipherData_;
    SecByteBlock key_;
};


template <typename DataType>
Encryptor<DataType>::Encryptor(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue, std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
    : DataProcessor<Encryptor<DataType>, DataType>(currentQueue, nextQueue)
    , cipherData_(std::make_unique<DataType>()) {
    setKey();
}

template <typename DataType>
void Encryptor<DataType>::processDataImpl() {
    // iv
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
    memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);
    // copy info that's not encrypted
    cipherData_->header.type = this->data_->header.type;
    cipherData_->header.length = this->data_->header.length;
    // encryption
    try {
        CBC_Mode<AES>::Encryption encryption;
        encryption.SetKeyWithIV(key_, key_.size(), iv, sizeof(iv));
        ArraySink arraySink(&cipherData_->data[0],
                            this->data_->header.length + AES::BLOCKSIZE);
        // TODO ??? Not sure that's correct
        ArraySource(&this->data_->data[0],
                    this->data_->header.length,
                    true,
                    new StreamTransformationFilter(encryption, new Redirector(arraySink)));
        // Set cipher text length now that its known
        cipherData_->header.length = arraySink.TotalPutLength();
    } catch (const CryptoPP::Exception& e) {
        std::cerr << e.what() << std::endl;
        spdlog::error("Encryptor couldn't encrypt data");
    }

    // need to change cipherData_ with data_ because DataProcessor::notifyComplete()
    // uses data_ field
    std::swap(this->data_, cipherData_);

    if (this->data_->header.type == Header::Type::Exit) {
        this->isProcessDone_ = true;
    }
}

template <typename DataType>
void Encryptor<DataType>::setKey() {
    const char* envKey = std::getenv("myKey");
    if (envKey) {
        key_.Assign(reinterpret_cast<const CryptoPP::byte*>(envKey),
                    strlen(envKey));
#ifdef DEBUG
        spdlog::info("Key for encryption is taken from environment variables");
#endif
    } else {
        std::string hardCodedKey("5E462EA6BD40B083F5F2C4B810A07230");
        key_.Assign(reinterpret_cast<const CryptoPP::byte*>(hardCodedKey.data()),
                    hardCodedKey.size());
#ifdef DEBUG
        spdlog::info("Use hardcoded key in Encryptor");
#endif
    }
}

}
#endif  // NETWORKING_ENCRYPTOR_H

//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_DECRYPTOR_H
#define NETWORKING_DECRYPTOR_H

#include "data_processor_interface.h"
#include "my_packet.h"
#include "pch.h"

namespace network {
    using namespace CryptoPP;

    const std::string DecryptorKey("Decryptor");

    // purpose of this class is to get encrypted data -> decrypt -> set to the next queue decrypted
    // packet
    template <typename DataType>
    class Decryptor : public DataProcessor<Decryptor<DataType>, DataType> {
        Decryptor(const Decryptor&) = delete;
        Decryptor(Decryptor&&) = delete;
        Decryptor& operator=(const Decryptor&) = delete;
        Decryptor& operator=(Decryptor&&) = delete;

    public:
        explicit Decryptor(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                           std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue);

        // get encrypted data -> decrypt to plain text
        // questions:
        // can std::FILE work with CryptoPP::byte
        void processDataImpl();

    protected:
        void setKey();

        std::unique_ptr<DataType> plainData_;
        SecByteBlock key_;
    };

    template <typename DataType>
    Decryptor<DataType>::Decryptor(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                                   std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
        : DataProcessor<Decryptor<DataType>, DataType>(currentQueue, nextQueue),
          plainData_(std::make_unique<DataType>()) {
        setKey();
    }

    template <typename DataType>
    void Decryptor<DataType>::processDataImpl() {
        // iv
        CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
        memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

        plainData_->header.type = this->data_->header.type;
        plainData_->header.length = this->data_->header.length;

        try {
            CBC_Mode<AES>::Decryption decryption;
            decryption.SetKeyWithIV(key_, key_.size(), iv, sizeof(iv));
            ArraySink arraySink(&plainData_->data[0], this->data_->header.length);
            ArraySource(&this->data_->data[0], this->data_->header.length, true,
                        new StreamTransformationFilter(decryption, new Redirector(arraySink)));
            plainData_->header.length = arraySink.TotalPutLength();
        } catch (const CryptoPP::Exception& e) {
            std::cerr << e.what() << std::endl;
            spdlog::error("Decryptor couldn't decrypt data");
        }

        // need to change plainData_ with data_ because DataProcessor::notifyComplete()
        // uses data_ field
        std::swap(this->data_, plainData_);

        if (this->data_->header.type == Header::Type::Exit) {
            this->isProcessDone_ = true;
        }
    }

    template <typename DataType>
    void Decryptor<DataType>::setKey() {
        const char* envKey = std::getenv("myKey");
        if (envKey) {
            key_.Assign(reinterpret_cast<const CryptoPP::byte*>(envKey), strlen(envKey));
#ifdef DEBUG
            spdlog::info("Key for encryption is taken from environment variables");
#endif
        } else {
            // only for education purposes
            // Bad, I know ...
            std::string hardCodedKey("5E462EA6BD40B083F5F2C4B810A07230");
            key_.Assign(reinterpret_cast<const CryptoPP::byte*>(hardCodedKey.data()),
                        hardCodedKey.size());
#ifdef DEBUG
            spdlog::info("Use hardcoded key in Decryptor");
#endif
        }
    }

}  // namespace network

#endif  // NETWORKING_DECRYPTOR_H

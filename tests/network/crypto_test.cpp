
#include "crypto_test.h"

#include <iostream>

#include "encryptor.h"
#include "decryptor.h"
#include "gtest/gtest.h"
#include "my_packet.h"
#include "thread_safe_queue.h"

using namespace std;
using namespace testing;
using namespace network;

namespace {

using CryptoPacket = MyPacket<CryptoPP::byte>;

std::vector<std::shared_ptr<ThreadSafeQueue<CryptoPacket>>> getQueue() {
    return {std::make_shared<ThreadSafeQueue<CryptoPacket>>(true),
            std::make_shared<ThreadSafeQueue<CryptoPacket>>(false)};
}

// finish Decryptor (processFunc
// test plan: prepare some encrypted 'HelloWorld' packet
// make mock class with decryptor
// put prepared packet
// test expected results(data, len etc)

//verify that output packet can be used in file writer

TEST(EncryptorTest, test_ctr) {
    auto tsQueue = getQueue();
    Encryptor<CryptoPacket> encryptor(tsQueue[0], tsQueue[1]);
}

TEST(DecryptorTest, test_ctr) {
    auto tsQueue = getQueue();
    Decryptor<CryptoPacket> decryptor(tsQueue[0], tsQueue[1]);
}

std::string str("Hello World!");

template <typename DataType>
class EncryptorMock: public Encryptor<DataType> {
public:
    EncryptorMock(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                  std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
        : Encryptor<DataType>(currentQueue, nextQueue) {
        this->data_ = std::make_unique<DataType>();
        this->data_->header.type = Header::Type::FileData;

        this->data_->header.length = str.size();
        std::copy(str.begin(), str.end(), this->data_->data.begin());
    }

    void setData(std::unique_ptr<DataType>&& data) {
        this->data_ = std::move(data);
    }

    std::unique_ptr<DataType> getData() {
        return std::move(this->data_);
    }
};

TEST(EncryptorTest, test_process) {
    auto tsQueue = getQueue();
    EncryptorMock<CryptoPacket> encryptor(tsQueue[0], tsQueue[1]);
    EXPECT_NO_THROW(encryptor.processData());
}

template <typename DataType>
class DecryptorMock: public Decryptor<DataType> {
public:
    DecryptorMock(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                  std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
        : Decryptor<DataType>(currentQueue, nextQueue) {
        this->data_ = std::make_unique<DataType>();
    }

    std::unique_ptr<DataType> getData() {
        return std::move(this->data_);
    }

    void setData(std::unique_ptr<DataType>&& data) {
        this->data_ = std::move(data);
    }
};

TEST(DecryptorTest, test_process) {
    auto tsQueue = getQueue();
    EncryptorMock<CryptoPacket> encryptor(tsQueue[0], tsQueue[1]);
    encryptor.processData();

    DecryptorMock<CryptoPacket> decryptor(tsQueue[1], tsQueue[0]);
    decryptor.setData(encryptor.getData());
    decryptor.processData();
    auto res = decryptor.getData();

    for (int i = 0; i < str.size(); i++) {
        EXPECT_EQ(str[i], res->data[i]);
    }
}

}
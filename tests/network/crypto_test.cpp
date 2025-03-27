
#include "crypto_test.h"

#include <iostream>

#include "decryptor.h"
#include "defs_mock.h"
#include "encryptor.h"
#include "gtest/gtest.h"
#include "my_packet.h"
#include "thread_safe_queue.h"

using namespace std;
using namespace testing;
using namespace network;

std::string gHelloWorldStr("Hello World!");

std::vector<CryptoPacket> gAllPacketVec{{Header::Type::Ack, 0, ""},
                                        {Header::Type::Nack, 0, ""},
                                        {Header::Type::FileName, 7, "tmp.txt"},
                                        {Header::Type::FileData, 12, "Hello World!"},
                                        {Header::Type::Hash, 3, "!@#"},
                                        {Header::Type::Exit, 0, ""}};

namespace {

    // finish Decryptor (processFunc
    // test plan: prepare some encrypted 'HelloWorld' packet
    // make mock class with decryptor
    // put prepared packet
    // test expected results(data, len etc)

    // verify that output packet can be used in file writer

    TEST(EncryptorTest, test_ctr) {
        auto tsQueue = get2Queue<CryptoPacket>();
        Encryptor<CryptoPacket> encryptor(tsQueue[0], tsQueue[1]);
    }

    TEST(DecryptorTest, test_ctr) {
        auto tsQueue = get2Queue<CryptoPacket>();
        Decryptor<CryptoPacket> decryptor(tsQueue[0], tsQueue[1]);
    }

    TEST(EncryptorTest, test_process) {
        auto tsQueue = get2Queue<CryptoPacket>();
        EncryptorMock<CryptoPacket> encryptor(tsQueue[0], tsQueue[1]);
        EXPECT_NO_THROW(encryptor.processData());
    }

    template <typename DataType>
    class DecryptorMock : public Decryptor<DataType> {
    public:
        DecryptorMock(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                      std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
            : Decryptor<DataType>(currentQueue, nextQueue) {
            this->data_ = std::make_unique<DataType>();
        }

        std::unique_ptr<DataType> getData() { return std::move(this->data_); }

        void setData(std::unique_ptr<DataType>&& data) { this->data_ = std::move(data); }
    };

    TEST(DecryptorTest, test_process) {
        auto tsQueue = get2Queue<CryptoPacket>();
        EncryptorMock<CryptoPacket> encryptor(tsQueue[0], tsQueue[1]);
        encryptor.processData();

        DecryptorMock<CryptoPacket> decryptor(tsQueue[1], tsQueue[0]);
        decryptor.setData(encryptor.getData());
        decryptor.processData();
        auto res = decryptor.getData();

        for (int i = 0; i < gHelloWorldStr.size(); i++) {
            EXPECT_EQ(gHelloWorldStr[i], res->data[i]);
        }
    }

    TEST(TestHeader, test_operator) { EXPECT_NE(gAllPacketVec[0].header, gAllPacketVec[1].header); }

    // Test all packages, test a few iterations of encrypt/decrypt data
    // 1. create generator of all type of packages - send further one by one
    // 2. send to encryptor - encrypt - send further
    // 3. send to decryptor - decrypt - compare with packets from first step

    TEST(CryptoTest, test_packages) {
        // create queues 3
        // PacketGeneratorMock
        // process - set (ack/nack/name/data/hash/exit - is done)
        auto tsQueues = get3Queue<CryptoPacket>();
        std::vector<std::thread> threads;

        // PacketGenerator
        threads.emplace_back([&]() {
            PacketGeneratorMock<CryptoPacket> packetGeneratorMock(tsQueues[0], tsQueues[1]);
            for (const auto pack : gAllPacketVec) {
                auto curPack = make_unique<CryptoPacket>(pack);
                packetGeneratorMock.waitNextData();
                packetGeneratorMock.set(std::move(curPack));
                packetGeneratorMock.notifyComplete();
            }
        });

        // Encryptor
        threads.emplace_back([&]() {
            Encryptor<CryptoPacket> encryptor(tsQueues[1], tsQueues[2]);
            while (not encryptor.isDone()) {
                encryptor.waitNextData();
                encryptor.processData();
                encryptor.notifyComplete();
            }

            EXPECT_TRUE(encryptor.isDone());
        });

        // Decryptor
        threads.emplace_back([&]() {
            DecryptorMock<CryptoPacket> decryptor(tsQueues[2], tsQueues[0]);

            for (const auto pack : gAllPacketVec) {
                decryptor.waitNextData();
                decryptor.processData();
                auto processPack = decryptor.getData();
                EXPECT_EQ(processPack->header, pack.header);
                if (processPack->header.length > 0) {
                    std::string curStr(processPack->data.begin(),
                                       processPack->data.begin() + processPack->header.length);
                    std::string expectedStr(pack.data.begin(),
                                            pack.data.begin() + pack.header.length);
                    EXPECT_EQ(curStr, expectedStr);
                }
                decryptor.setData(std::move(processPack));
                decryptor.notifyComplete();
            }

            EXPECT_TRUE(decryptor.isDone());
        });

        for (auto& th : threads) {
            // if joinable
            th.join();
        }
    }

}
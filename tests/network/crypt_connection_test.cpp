#include "crypt_connection_test.h"

#include "pch.h"
#include "my_packet.h"
#include "thread_safe_queue.h"
#include "connection.h"
#include "defs_mock.h"

using namespace std;
using namespace testing;
using namespace network;

// client
// thread 1 generate
// thread 2 encrypt
// thread 3 send via socket

// server
// thread 1 get from socket
// thread 2 decrypt and compare


// packs: Data + exit Pack



void runClient() {
    auto tsQueues = get3Queue<CryptoPacket>();

    std::vector<std::thread> threads;

    // create packet generator
    auto packetGen = std::make_unique<PacketGeneratorMock<CryptoPacket>>(tsQueues[0], tsQueues[1]);

    // create encryptor
    auto encryptor = std::make_unique<Encryptor<CryptoPacket>>(tsQueues[1], tsQueues[2]);


    // create connection entity
    boost::asio::io_context context;
    boost::asio::ip::tcp::endpoint endpoint(address::from_string(ipDefualt),
                                            static_cast<port_type>(portDefualt));
    boost::asio::ip::tcp::socket socket(context);
    boost::system::error_code errorCode;
    socket.connect(endpoint, errorCode);
    auto connection = std::make_unique<Connection<CryptoPacket>>(std::move(socket), tsQueues[2], tsQueues[0]);

    threads.emplace_back([packetGen = std::move(packetGen)]{
        auto packs = getAlphabetPacks<CryptoPacket>();
        for (auto& pack: packs) {
            packetGen->waitNextData();
            packetGen->set(std::move(pack));
            packetGen->notifyComplete();
        }
        auto exitPack = std::make_unique<CryptoPacket>(CryptoPacket{Header::Type::Exit, 0, ""});
        packetGen->waitNextData();
        packetGen->set(std::move(exitPack));
        packetGen->notifyComplete();
    });

    // encrypt
    threads.emplace_back([encryptor = std::move(encryptor)]{
        while (not encryptor->isDone()) {
            encryptor->waitNextData();
            encryptor->processData();
            encryptor->notifyComplete();
        }
    });

    threads.emplace_back([connection = std::move(connection)]{
        while (not connection->isDone()) {
            connection->waitNextData();
            connection->processData();
            connection->notifyComplete();
        }

    });

    for (auto& th: threads) {
        // if joinable
        th.join();
    }
}

void runServer() {
    auto tsQueues = get3Queue<CryptoPacket>();
    std::vector<std::thread> threads;

    // connection
    io_context ioContext_;
    ip::tcp::acceptor acceptor_(ioContext_, tcp::endpoint(make_address(ipDefualt), portDefualt));
    tcp::socket socket(ioContext_);
    acceptor_.accept(socket);
    auto connection = std::make_unique<Connection<CryptoPacket>>(std::move(socket), tsQueues[0], tsQueues[1]);
    // connection
    threads.emplace_back([connection = std::move(connection)]{
        while (not connection->isDone()) {
            connection->waitNextData();
            connection->processData();
            connection->notifyComplete();
        }
    });

    // decryptor
    auto decryptor = std::make_unique<Decryptor<CryptoPacket>>(tsQueues[1], tsQueues[2]);

    // collector
    auto packetCollector = std::make_unique<PacketCollectorMock<CryptoPacket>>(tsQueues[2], tsQueues[0]);


    threads.emplace_back([decryptor = std::move(decryptor)]{
        while (not decryptor->isDone()) {
            decryptor->waitNextData();
            decryptor->processData();
            decryptor->notifyComplete();
        }
    });


    threads.emplace_back([&packetCollector]{
        while (not packetCollector->isDone()) {
            packetCollector->waitNextData();
            packetCollector->collect();
            packetCollector->notifyComplete();
        }
    });

    // compare results with alpfaPacks
    for (auto& th: threads) {
        // if joinable
        th.join();
    }

    auto expectedPacks = getAlphabetPacks<CryptoPacket>();
    for (int i = 0; i < packetCollector->dataVec.size(); i++) {
        EXPECT_EQ(packetCollector->dataVec[i]->header, expectedPacks[i]->header);
        EXPECT_EQ(packetCollector->dataVec[i]->data[0], expectedPacks[i]->data[0]);
    }

}

TEST(CryptoConnectionTest, test_sendReceiveCryptoPacks) {

    std::thread serverThread(runServer);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread clientThread(runClient);

    if (serverThread.joinable()) {
        serverThread.join();
    }
    if (clientThread.joinable()) {
        clientThread.join();
    }
}
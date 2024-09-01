//
// Created by Renat_Khatymov on 8/25/2024.
//

#include "connection_test.h"

#include <iostream>

#include "my_packet.h"
#include "thread_safe_queue.h"
#include "connection.h"

#include "gtest/gtest.h"

using namespace std;
using namespace testing;
using namespace network;

// input : socket, q_cur, q_next
// socket create


// connection - get prepared socket
// add some logic
    // 1. server connection: process:
        // read data ? add watch dog???
        // write ack data
    // 2. client connection: process:
        // write data
        // read data ? add watch dog???

// connection can be for server and client

// do not forget of edge cases
// test plan:
// Preparation: make functions that create sockets for client and server
// server & client
// 1. test constructor/destructor - just initiate connection for server&client
    // a. ctr
    // b. dctr
// verify for mem leak
// 2. test waitNextData()
    // a. test it's actually waiting - watch dog
    // b. input -> data in some other component
    // we got this data (need client) | create mock class
// 3. test notifyComplete()
    // a. set data for next component -> verify that this data was accepted
// 4. test processDataImpl()
    // a. read test
    // b. write test
    // c. read/write

// 5. test whole cycle


//1. Black box
//2. define components
//3. POC
//4. define interfaces (SOLID/design patterns)
//5. refactoring (SOLID/design patterns)
//6. unit tests
//7. make it fast



// void createClientSocket(...)
// Class ConnectionMock

using namespace boost::asio;
using namespace boost::asio::ip;

//class ConnectionTest: public ::testing::Test {
//public:
//    ConnectionTest(){};
//
//    void SetUp() override {
//        tsQueues = {std::make_shared<ThreadSafeQueue<MyPacket<char>>>(true),
//                    std::make_shared<ThreadSafeQueue<MyPacket<char>>>(false)};
//    }
//
//    void TearDown() override {
//
//    }
//
//    MyPacket<char> packet;
//    std::vector<std::shared_ptr<ThreadSafeQueue<MyPacket<char>>>> tsQueues;
//};
namespace {

std::vector<std::shared_ptr<ThreadSafeQueue<MyPacket<char>>>> get2Queue() {
    return {std::make_shared<ThreadSafeQueue<MyPacket<char>>>(true), std::make_shared<ThreadSafeQueue<MyPacket<char>>>(false)};
}

TEST(ConnectionTest, test_ctr) {
    auto tsQueues = get2Queue();
    boost::asio::io_context io_context_;
    tcp::socket someSocket(io_context_);
    EXPECT_NO_THROW(Connection<MyPacket<char>> connection(Mode::Server, std::move(someSocket), tsQueues[0], tsQueues[1]));
}

// TODO
TEST(ConnectionTest, test_waitNextData) {}

// TODO
TEST(ConnectionTest, test_notifyComplete) {}

constexpr std::string ipDefualt("127.0.0.1");
constexpr uint portDefualt = 1234;

std::unique_ptr<MyPacket<char>> getPacketWithFileData() {
    std::unique_ptr<MyPacket<char>> packetWithFileData = std::make_unique<MyPacket<char>>();

    packetWithFileData->header.type = Header::Type::FileData;
    packetWithFileData->header.length = 1;
    packetWithFileData->data[0] = 'a';

    return std::move(packetWithFileData);
}

template <typename T>
class ConnectionMock : public Connection<T> {
public:
    ConnectionMock(const Mode mode, boost::asio::ip::tcp::socket socket, std::shared_ptr<ThreadSafeQueue<T>> currentQueue,
                   std::shared_ptr<ThreadSafeQueue<T>> nextQueue)
        : Connection<T>(mode, std::move(socket), currentQueue, nextQueue){};

    void setData(std::unique_ptr<T>&& data) { this->data_ = std::move(data); }

    std::unique_ptr<T> getData() { return std::move(this->data_); }
};

struct DummyServer {
    DummyServer(const std::string& ip = ipDefualt, const uint port = portDefualt)
        : acceptor_(io_context_, ip::tcp::endpoint(boost::asio::ip::make_address(ip), port)) {}

    void start() {
        tcp::socket socket(io_context_);
        acceptor_.accept(socket);
        // won't be used
        auto tsQueues = get2Queue();
        ConnectionMock<MyPacket<char>> connectionMock(Mode::Server, std::move(socket), tsQueues[0], tsQueues[1]);
        // allocate memory for packet
        connectionMock.setData(std::move(make_unique<MyPacket<char>>()));
        // connection should accept data from socket
        connectionMock.processData();
        // get result for verification
        auto resPacket = connectionMock.getData();
        auto expectedPacket = getPacketWithFileData();

        EXPECT_EQ(resPacket->header.type, expectedPacket->header.type);
        EXPECT_EQ(resPacket->header.length, expectedPacket->header.length);
        EXPECT_EQ(resPacket->data[0], expectedPacket->data[0]);
    }

    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
};

// 4. test processDataImpl()
// a. read test
// b. write test
// c. read/write

// finish DummyServer

// create a client
// separate thread and function for the client

// The client should attach to the endpoint, create socket and send some package to a server, as simple as it is
class DummyClient {
    DummyClient(const DummyClient&) = delete;
    DummyClient(DummyClient&&) = delete;
    DummyClient operator=(const DummyClient&) = delete;
    DummyClient operator=(DummyClient&&) = delete;

public:
    DummyClient(const std::string& ip = ipDefualt, const uint port = portDefualt)
        : endpoint_{address::from_string(ip),
          static_cast<port_type>(port)},
          socket_{context_} {}

    void start() {
        boost::system::error_code errorCode;
        socket_.connect(endpoint_, errorCode);
        auto packetWithFileData = getPacketWithFileData();
        EXPECT_TRUE(Connection<MyPacket<char>>::write(socket_, packetWithFileData, errorCode));
        EXPECT_TRUE(Connection<MyPacket<char>>::read(socket_, packetWithFileData, errorCode));
        EXPECT_EQ(packetWithFileData->header.type, Connection<MyPacket<char>>::ackPacket.header.type);
    }

    //! \brief asio context handles the data transfer...
    boost::asio::io_context context_;
    //! \brief using to resolve hostname/ip-address into tangiable physical address
    boost::asio::ip::tcp::endpoint endpoint_;
    //! \brief socket allows us to connect to the server
    boost::asio::ip::tcp::socket socket_;
};

TEST(ConnectionTest, test_processDataImplServer) {
    std::thread serverThread([]() {
        DummyServer dummyServer;
        dummyServer.start();
    });

    // server should start first
    this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread clientThread([]() {
        DummyClient dummyClient;
        dummyClient.start();
    });

    serverThread.join();
    clientThread.join();
}
}
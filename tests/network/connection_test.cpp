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


// TODO make these tests - fixture tests
TEST(ConnectionTest, test_ctr) {
    std::vector<std::shared_ptr<ThreadSafeQueue<MyPacket<char>>>> tsQueues = {std::make_shared<ThreadSafeQueue<MyPacket<char>>>(true),
                                     std::make_shared<ThreadSafeQueue<MyPacket<char>>>(false)};
    EXPECT_NO_THROW([&tsQueues]() {
        boost::asio::io_context io_context_;
        tcp::socket someSocket(io_context_);
        Connection<MyPacket<char>> connection(Mode::Server, std::move(someSocket), tsQueues[0], tsQueues[1]);
    });
}

// TODO
TEST(ConnectionTest, test_waitNextData) {

}

// TODO
TEST(ConnectionTest, test_notifyComplete) {

}


struct DummyServer {
    DummyServer(const std::string& ip = "127.0.0.1", const uint port = 1234)
        :acceptor_(io_context_, ip::tcp::endpoint(boost::asio::ip::make_address(ip), port)) {}


    void tmp() {
        tcp::socket socket(io_context_);
        acceptor_.accept(socket);

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
    // The client should create some packages and send it to server

// connection accept data
// create mock class to see the data
// what we need to check: all package types
// set a flag is done
//

TEST(ConnectionTest, test_processDataImplServer) {

}

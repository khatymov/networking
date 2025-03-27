//
// Created by Renat_Khatymov on 8/25/2024.
//

#include "connection_test.h"

#include <iostream>

#include "my_packet.h"
#include "thread_safe_queue.h"
#include "connection.h"
#include "defs_mock.h"

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

TEST(ConnectionTest, test_ctr) {
    auto tsQueues = get2Queue<MyPacket<char>>();
    boost::asio::io_context io_context_;
    tcp::socket someSocket(io_context_);
    EXPECT_NO_THROW(Connection<MyPacket<char>> connection(std::move(someSocket), tsQueues[0], tsQueues[1]));
}

// TODO
TEST(ConnectionTest, test_waitNextData) {}

// TODO
TEST(ConnectionTest, test_notifyComplete) {}

TEST(ConnectionTest, test_processDataImplServer) {
    std::thread serverThread([]() {
        DummyServer<MyPacket<char>> dummyServer;
        dummyServer.start();
    });

    // server should start first
    this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread clientThread([]() {
        DummyClient<MyPacket<char>> dummyClient;
        dummyClient.start();
    });

    serverThread.join();
    clientThread.join();
}

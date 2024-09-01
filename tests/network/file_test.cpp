//
// Created by Renat_Khatymov on 9/1/2024.
//

#include "file_test.h"

#include <iostream>

#include "gtest/gtest.h"
#include "my_packet.h"
#include "thread_safe_queue.h"

#include "file_writer.h"
#include "file_reader.h"

using namespace std;
using namespace testing;
using namespace network;


namespace {

using CryptoPacket = MyPacket<CryptoPP::byte>;

std::vector<std::shared_ptr<ThreadSafeQueue<CryptoPacket>>> get2Queue() {
    return {std::make_shared<ThreadSafeQueue<CryptoPacket>>(true),
            std::make_shared<ThreadSafeQueue<CryptoPacket>>(false)};
}

TEST(FileReaderTest, test_ctr) {
    auto tsQueues = get2Queue();
//    EXPECT_NO_THROW();
}

TEST(FileWriterTest, test_ctr) {
    auto tsQueues = get2Queue();
    EXPECT_NO_THROW(FileWriter<CryptoPacket> fileWriter(tsQueues[0], tsQueues[1]));
}

// test plan for file writer
// create dummy packets as its done in crypto
// need to test
// 1. file creation:
//      a) file doesn't exist
//      b) file exists
// 2. file writing: a. compare file contents and what dummy packet has
//                  b. try to write if file didn't open
// 3. calculating hash: a. compare with pre-defined has (use linux tool)
//                      b.empty file
// 4. exit

TEST(FileWriterTest, test_process) {

}

}
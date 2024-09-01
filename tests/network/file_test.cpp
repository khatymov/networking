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

// delete file
void prepareEnv() {
    std::system("[[ -f tmp.txt ]] && rm tmp.txt");
}

std::string readFileIntoString(const std::string& filename) {
    // Open the file with ifstream in binary mode and at the end of the file
    std::ifstream file(filename, std::ios::binary);

    // Check if the file opened successfully
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    // Use a stringstream to read the file contents into a string
    std::ostringstream oss;
    oss << file.rdbuf();

    // Return the contents of the stringstream as a string
    return oss.str();
}

std::vector<CryptoPacket> packets{{Header::Type::FileName, 7, "tmp.txt"},
                                       {Header::Type::FileData, 12, "Hello World!"},
                                       {Header::Type::Hash, 64, "7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069"},
                                       {Header::Type::Exit, 0, ""}};
template <typename T>
class FileWriterMock: public FileWriter<T> {
public:
    explicit FileWriterMock(std::shared_ptr<ThreadSafeQueue<T>> currentQueue,
                   std::shared_ptr<ThreadSafeQueue<T>> nextQueue)
        : FileWriter<T>(currentQueue, nextQueue) {}

    std::unique_ptr<T> getData() {
        return std::move(this->data_);
    }

    void setData(std::unique_ptr<T>&& data) {
        this->data_ = std::move(data);
    }
};

TEST(FileWriterTest, test_cryptoProcess) {
    prepareEnv();
    auto tsQueue = get2Queue();
    FileWriterMock<CryptoPacket> fileWriter(tsQueue[0], tsQueue[1]);
    for (const auto curPack: packets) {
        fileWriter.setData(make_unique<CryptoPacket>(curPack));
        EXPECT_NO_THROW(fileWriter.processData());
        if (curPack.header.type == Header::Type::FileName) {
            auto expectedFile(string(curPack.data.begin(), curPack.data.begin() + curPack.header.length));
            EXPECT_TRUE(std::filesystem::exists(expectedFile));
        }
    }

    EXPECT_TRUE(fileWriter.isDone());

    auto fileName(string(packets[0].data.begin(), packets[0].data.begin() + packets[0].header.length));
    auto fileData = readFileIntoString(fileName);
    auto expectedData(string(packets[1].data.begin(), packets[1].data.begin() + packets[1].header.length));
    EXPECT_EQ(fileData, expectedData);
}

TEST(FileWriterTest, test_cryptoFileExists) {
    //Prepare env
    std::system("touch tmp.txt");

    auto tsQueue = get2Queue();
    FileWriterMock<CryptoPacket> fileWriter(tsQueue[0], tsQueue[1]);
    for (const auto curPack: packets) {
        fileWriter.setData(make_unique<CryptoPacket>(curPack));
        EXPECT_NO_THROW(fileWriter.processData());
    }
}

// test plan for file reader
// TODO:
TEST(FileReaderTest, test_ctr) {
    auto tsQueues = get2Queue();
    //    EXPECT_NO_THROW();
}

}
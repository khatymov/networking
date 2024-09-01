//
// Created by Renat_Khatymov on 9/1/2024.
//

#include "file_test.h"

#include <iostream>
#include <fstream>
#include <random>
#include <vector>

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
    std::system("[[ -f fileTest.txt ]] && rm fileTest.txt");
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

std::vector<CryptoPacket> packets{{Header::Type::FileName, 7, "fileTest.txt"},
                                       {Header::Type::FileData, 12, "Hello World!"},
                                       {Header::Type::Hash, 64, "7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069"},
                                       {Header::Type::Exit, 0, ""}};

std::vector<CryptoPacket> withMissedPackets0{{Header::Type::FileData, 12, "Hello World!"},
                                            {Header::Type::Hash, 64, "7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069"},
                                            {Header::Type::Exit, 0, ""}};

std::vector<CryptoPacket> withMissedPackets1{{Header::Type::FileName, 7, "fileTest.txt"},
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
    std::system("touch fileTest.txt");

    auto tsQueue = get2Queue();
    FileWriterMock<CryptoPacket> fileWriter(tsQueue[0], tsQueue[1]);
    for (const auto curPack: packets) {
        fileWriter.setData(make_unique<CryptoPacket>(curPack));
        EXPECT_NO_THROW(fileWriter.processData());
    }
}

TEST(FileWriterTest, test_cryptoMissedPacks) {
    //Prepare env
//    std::system("touch fileTest.txt");
//
//    EXPECT_THROW([](){
//        auto tsQueue = get2Queue();
//        FileWriterMock<CryptoPacket> fileWriter(tsQueue[0], tsQueue[1]);
//        for (const auto curPack: withMissedPackets0) {
//            fileWriter.setData(make_unique<CryptoPacket>(curPack));
//            fileWriter.processData();
//        };
//    }(),std::runtime_error);

    prepareEnv();

    EXPECT_THROW([](){
        auto tsQueue = get2Queue();
        FileWriterMock<CryptoPacket> fileWriter(tsQueue[0], tsQueue[1]);
        for (const auto curPack: withMissedPackets1) {
            fileWriter.setData(make_unique<CryptoPacket>(curPack));
            fileWriter.processData();
        };
    }(),std::runtime_error);

}

// test plan for file reader
// shared queues
// thread 1 (reader)
// read some file - send further
// thread 2 (writer)
// get data - write to file
// compare via diff

TEST(FileReaderTest, test_ctr) {
    auto tsQueues = get2Queue();
    EXPECT_NO_THROW(FileReader<CryptoPacket> fileReader("fileReadTest.txt",
                                                        tsQueues[0],
                                                        tsQueues[1]));
}



void generateRandomFile(const std::string& filename, size_t sizeInBytes) {
    // Open file stream for binary writing
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Could not open file for writing." << std::endl;
        return;
    }

    // Random number generator setup
    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_int_distribution<> distr(0, 255); // Define the range for bytes (0-255)

    // Buffer to hold random data (e.g., 1 MB buffer)
    const size_t bufferSize = 1024 * 1024;
    std::vector<char> buffer(bufferSize);

    size_t bytesWritten = 0;
    while (bytesWritten < sizeInBytes) {
        // Fill buffer with random bytes
        for (size_t i = 0; i < bufferSize && (bytesWritten + i) < sizeInBytes; ++i) {
            buffer[i] = static_cast<char>(distr(gen));
        }

        // Determine how much data to write this iteration
        size_t bytesToWrite = std::min(bufferSize, sizeInBytes - bytesWritten);

        // Write to the file
        outFile.write(buffer.data(), bytesToWrite);
        if (!outFile) {
            std::cerr << "Error: Could not write to file." << std::endl;
            return;
        }

        // Update bytes written
        bytesWritten += bytesToWrite;
    }

    outFile.close(); // Close the file stream

    if (outFile.good()) {
        std::cout << "File '" << filename << "' created with " << sizeInBytes << " bytes of random data." << std::endl;
    } else {
        std::cerr << "Error occurred during file creation." << std::endl;
    }
}


template <typename T>
class FileReaderMock: public FileReader<T> {
public:
    explicit FileReaderMock(const std::string& name,
                            std::shared_ptr<ThreadSafeQueue<T>> currentQueue,
                            std::shared_ptr<ThreadSafeQueue<T>> nextQueue)
        : FileReader<T>(name, currentQueue, nextQueue) {}

    std::unique_ptr<T> getData() {
        return std::move(this->data_);
    }

    void setData(std::unique_ptr<T>&& data) {
        this->data_ = std::move(data);
    }
};

TEST(FileReaderTest, test_process) {
    auto tsQueues = get2Queue();
    std::vector<std::thread> threads;

    std::string filename = "data.txt";
    size_t fileSize = 1 * 1024 * 1024; // 1 MB

    generateRandomFile(filename, fileSize);
    threads.emplace_back([&](){
        FileReaderMock<CryptoPacket> fileReader(filename,
                                            tsQueues[0],
                                            tsQueues[1]);

        // prepare: open file
        auto nameTmp = filename.c_str();
        CryptoPacket fnamePack({Header::Type::FileName, filename.size(), "data.txt"});
        fileReader.waitNextData();
        fileReader.setData(make_unique<CryptoPacket>(fnamePack));
        EXPECT_NO_THROW(fileReader.notifyComplete());

        while (not fileReader.isDone()) {
            fileReader.waitNextData();
            fileReader.processData();
            fileReader.notifyComplete();
        }

        CryptoPacket exitPack({Header::Type::Exit, 0, ""});
        fileReader.waitNextData();
        fileReader.setData(make_unique<CryptoPacket>(exitPack));
        EXPECT_NO_THROW(fileReader.notifyComplete());

    });

    threads.emplace_back([&](){
        FileWriterMock<CryptoPacket> fileWriter(tsQueues[1], tsQueues[0]);

        while (not fileWriter.isDone()) {
            fileWriter.waitNextData();
            fileWriter.processData();
            fileWriter.notifyComplete();
        }
    });

    for (auto& th: threads) {
        // if joinable
        th.join();
    }

}

}
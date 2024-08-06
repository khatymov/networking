#include "gtest/gtest.h"
#include "network_test.h"

#include <iostream>
#include <openssl/sha.h>
#include <sstream>

using namespace std;
using namespace testing;


/*
 * Network : socket
 *
 *
 * PacketManager - switchCaser
 * void handle()
 * {
 *     CryptoPacket* packet = nullprt;
 *     while(1)
 *         packet = get_available_buffer(Stage::manager)
 *         if packet == nullptr
 *            std::yield();
 *            continue;
 *         switch (packet.type())
 *         case: FileData -> transferBufferTo(Stage::FileHandler)
 *         case: CryptoData -> transferBufferTo(Stage::Cryptographer)
 *
 * }
 *
 * CryptoRanner cryptoWrapper
 * cryptoWrapper.run()
 * {
 *
 *     CryptoPacket* packet = nullprt;
 *     while(1)
 *         packet = get_available_buffer(Stage::manager)
 *         decrypt()
 * }
 */

#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <thread>

#include "common.h"
#include "packet.h"
#include "server.h"

using boost::asio::ip::tcp;

using namespace std;



class TestBase{
public:
    TestBase() = default;

    void get(std::shared_ptr<int> data) {
        std::cout << "Get Data sh_ptr counter before:" << data.use_count() << std::endl;
        val_ = data;
        std::cout << "Get Data sh_ptr counter after:" << data.use_count() << std::endl;
    }

    void process() {
        *val_ = *val_ + 10;
    }

    void set(std::shared_ptr<int> data) {
        std::cout << "Set Data sh_ptr counter before:" << data.use_count() << std::endl;
        data = std::move(val_);
        std::cout << "Set Data sh_ptr counter after:" << data.use_count() << std::endl;
//        data.push_back(std::move(val_));
    }

    std::shared_ptr<int> val_;
};



class TestPtr{
public:
    TestPtr() = default;

    void set(std::unique_ptr<int> data) {
        if (val_ == nullptr) {
            cout << "It works" << endl;
        }
//        std::cout << "Get Data sh_ptr counter before:" << data.use_count() << std::endl;
        val_ = std::move(data);

        if (val_ != nullptr) {
            cout << "It works" << endl;
        }
//        std::cout << "Get Data sh_ptr counter after:" << data.use_count() << std::endl;
    }

    void process() {
        *val_ = *val_ + 10;
    }

    std::unique_ptr<int> get() {
        return std::move(val_);
    }

    std::unique_ptr<int> val_;
};


TEST(test_network, test_unique_ptr) {
    std::vector<std::unique_ptr<int>> vecPtr;

    for (int i = 0; i < 3; i++) {
        vecPtr.push_back(make_unique<int>(i));
    }
    {
        TestPtr testBase;
        for (int i = 0; i < 3; i++) {
            cout << "Before: " << *vecPtr[i] << endl;
            testBase.set(std::move(vecPtr[i]));
            testBase.process();
            vecPtr[i] = std::move(testBase.get());
        }
    }

    for (int i = 0; i < 3; i++) {
        std::cout << "Final val:" << *vecPtr[i] << std::endl;
    }
}


//1. я пишу исключительно интерфейсы! не логику. ЧТО Я ОЖИДАЮ ОТ КЛАССА
//2. я пишу в расчете на то, что я переиспользую этот класс
//3. опиши все параметры своих методов + граничные случаи своих параметров.
//        что сделает set если пришел null,
//
//    https://github.com/zproksi/bpatch/blob/master/srcbpatch/streamreplacer.h#L34
//
//https://github.com/zproksi/bpatch/blob/master/srcbpatch/actionscollection.cpp#L279
//class DataFlow {
//public:
//    //
//    //    как ждать, сколько ждать, вне зависимости от типа пакета
//    std::unique_ptr<IPacket> getPacket() {
//        return std::unique_ptr<IPacket>;
//    }
//
//    //set packet for the next mode
//    //    закидывает в очередь
//    void setPacket(std::unique_ptr<IPacket>&& packet) {
//    }
//
//private:
//    //    std::unique_ptr<IPacket> targetOfCryptoPacket;
//    std::queue<std::unique_ptr<IPacket>> queue;
//    std::mutex _mutex;
//};




/*
 *
 * MY PACKET TEST
 *
 *
 */


#include "my_packet.h"

TEST(test_packet, test_packet_template)
{
    MyPacket<char> myPacket;
    EXPECT_EQ(PACKET_DATA_SIZE, myPacket.data.size());
    EXPECT_EQ(myPacket.header.type, Header::Type::Ack);
    EXPECT_EQ(myPacket.header.length, 0);

    MyPacket<CryptoPP::byte> cryptoPacket;
    EXPECT_EQ(PACKET_DATA_SIZE + CryptoPP::AES::BLOCKSIZE, cryptoPacket.data.size());
    EXPECT_EQ(cryptoPacket.header.type, Header::Type::Ack);
    EXPECT_EQ(cryptoPacket.header.length, 0);
}

template<typename PacketType = MyPacket<char>>
class PacketFlow {
public:
    enum class Stage : uint32_t {
        Socket,
        Crypto,
        File
    };


    explicit PacketFlow() {
        for (size_t i = 0; i < packetNum_; i++) {
            socketQueue_.push(std::make_unique<PacketType>());
        }
    }

    std::unique_ptr<PacketType> getPacket(const Stage& stage) {
        std::unique_ptr<PacketType> packet;
        auto* currentQueue = getQueueForStage(stage);
        std::lock_guard<std::mutex> lockGuard(mutex_);
        if (not currentQueue->empty()) {
            packet = std::move(currentQueue->front());
            currentQueue->pop();
        }

        return std::move(packet);
    }

    void setPacketForNextStage(const Stage& stage, std::unique_ptr<PacketType> packet) {
        auto* nextQueue = getQueueForStage(stage);
        std::lock_guard<std::mutex> lockGuard(mutex_);
        nextQueue->push(std::move(packet));
    }

    //mainly for test purposes
    [[maybe_unused]] auto getPacketsAmount() const {
        return packetNum_;
    }


    //mainly for test purposes
    [[maybe_unused]] auto getPacketsAmountInQueues() const {
        return socketQueue_.size() + fileQueue_.size() + cryptoQueue_.size();
    }

protected:

    static constexpr size_t packetNum_ = 3;

    std::queue<std::unique_ptr<PacketType>>* getQueueForStage(const Stage& stage) {
        switch (stage) {
            case (Stage::Socket): {
                return &socketQueue_;
            };
            case (Stage::Crypto): {
                return &cryptoQueue_;
            };
            case (Stage::File): {
                return &fileQueue_;
            };
            default:
                std::logic_error("Wrong type");
        }

        std::logic_error("Wrong type");
    }


//    std::array<PacketType, packetNum_> packets_;
    std::queue<std::unique_ptr<PacketType>> socketQueue_;
    std::queue<std::unique_ptr<PacketType>> fileQueue_;
    std::queue<std::unique_ptr<PacketType>> cryptoQueue_;
    std::mutex mutex_;
};

/*
 *
 * PACKET FLOW TEST
 *
 *
 */

TEST(test_packet_rotator, test_def_packet_constructor) {
    std::shared_ptr<PacketFlow<MyPacket<char>>> packetFlow = std::make_shared<PacketFlow<MyPacket<char>>>();
}

TEST(test_packet_rotator, test_crypto_packet_constructor) {
    std::shared_ptr<PacketFlow<MyPacket<CryptoPP::byte>>> packetFlow = std::make_shared<PacketFlow<MyPacket<CryptoPP::byte>>>();
}

TEST(test_packet_rotator, test_getPacket) {
    std::shared_ptr<PacketFlow<MyPacket<char>>> packetFlow = std::make_shared<PacketFlow<MyPacket<char>>>();

    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        std::unique_ptr<MyPacket<char>> packet = packetFlow->getPacket(PacketFlow<MyPacket<char>>::Stage::Socket);
        EXPECT_TRUE(packet != nullptr);
    }

    EXPECT_EQ(packetFlow->getPacket(PacketFlow<MyPacket<char>>::Stage::Socket), nullptr);
}

TEST(test_packet_rotator, test_setPacketForNextStage) {
    std::shared_ptr<PacketFlow<MyPacket<char>>> packetFlow = std::make_shared<PacketFlow<MyPacket<char>>>();

    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        std::unique_ptr<MyPacket<char>> packet = packetFlow->getPacket(PacketFlow<MyPacket<char>>::Stage::Socket);
        packet->header.type = Header::Type::FileData;
        packetFlow->setPacketForNextStage(PacketFlow<MyPacket<char>>::Stage::File, std::move(packet));
    }

    EXPECT_EQ(packetFlow->getPacketsAmount(), packetFlow->getPacketsAmountInQueues());

    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        std::unique_ptr<MyPacket<char>> packet = packetFlow->getPacket(PacketFlow<MyPacket<char>>::Stage::File);
        EXPECT_EQ(packet->header.type, Header::Type::FileData);
    }

    EXPECT_EQ(packetFlow->getPacket(PacketFlow<MyPacket<char>>::Stage::File), nullptr);
}


TEST(test_packet_rotator, test_allStages) {
    std::shared_ptr<PacketFlow<MyPacket<CryptoPP::byte>>> packetFlow = std::make_shared<PacketFlow<MyPacket<CryptoPP::byte>>>();

    //SOCKET -> CRYPTO
    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        std::unique_ptr<MyPacket<CryptoPP::byte>> packet = packetFlow->getPacket(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::Socket);
        packet->header.type = Header::Type::CryptoData;
        packet->data = {static_cast<CryptoPP::byte>(i)};
        packetFlow->setPacketForNextStage(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::Crypto, std::move(packet));
    }

    EXPECT_EQ(packetFlow->getPacketsAmount(), packetFlow->getPacketsAmountInQueues());

    //CRYPTO -> FILE
    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        std::unique_ptr<MyPacket<CryptoPP::byte>> packet = packetFlow->getPacket(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::Crypto);
        packet->header.type = Header::Type::FileData;
        packetFlow->setPacketForNextStage(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::File, std::move(packet));
    }

    EXPECT_EQ(packetFlow->getPacketsAmount(), packetFlow->getPacketsAmountInQueues());

    //FILE -> SOCKET
    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        std::unique_ptr<MyPacket<CryptoPP::byte>> packet = packetFlow->getPacket(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::File);
        EXPECT_EQ(packet->data[0], i);
        packetFlow->setPacketForNextStage(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::Socket, std::move(packet));
    }

    EXPECT_EQ(packetFlow->getPacketsAmount(), packetFlow->getPacketsAmountInQueues());
}

/*
 *
 * File TEST
 *
 *
 */
template <typename Derived>
class IStreamProcessor {
public:
    void waitNextData() {
        (static_cast<Derived*>(this)->waitNextData());
    }
    void processData() {
        (static_cast<Derived*>(this)->processData());
    }
    void notifyComplete() {
        (static_cast<Derived*>(this)->notifyComplete());
    }
};

template <typename T = MyPacket<char>>
class FileWriter: public IStreamProcessor<FileWriter<T>> {
public:
    explicit FileWriter(std::shared_ptr<PacketFlow<T>> packetFlow)
        :packetFlow_(packetFlow) {
    }
    ~FileWriter() {
        if (file_ != nullptr) {
            //            std::fclose(file_);
            file_ = nullptr;
        }
    }

    void waitNextData() {
        while (!(packet_ = packetFlow_->getPacket(PacketFlow<T>::Stage::File))) {
            std::this_thread::yield();
            std::cout << "waitNextData() queue File is empty" << std::endl;
            continue;
        }
        std::cout << "waitNextData() got the packet" << std::endl;
    }

    void processData() {
        switch (packet_->header.type) {
            case (Header::Type::FileName): {
                std::string fileName(packet_->data.data(), packet_->header.length);
                if (fileName.find('/') != string::npos) {
                    size_t lastSlashIndx = fileName.rfind('/');
                    fileName = fileName.substr(lastSlashIndx + 1, fileName.size());
                }

                if (isFileExist(fileName)) {
                    spdlog::debug("File with the name {} exists.", fileName);
                    fileName = getUniqueName(fileName);
                }

                spdlog::debug("Open a file:  {}", fileName);
                fileName_ = fileName;
                file_ = std::fopen(fileName.c_str(), "w");
                break;
            };
            case (Header::Type::FileData): {
                spdlog::info("Packet::Type::FileData: {}", packet_->header.length);
                auto sz = std::fwrite(&packet_->data,  sizeof(char), packet_->header.length, file_);
                std::cout << "processData() bytes written: " << sz << std::endl;
                break;
            };
            case (Header::Type::Hash): {
                spdlog::info("Generate a hash from file");
                // Hash of our file
                auto hash = getFileHash(fileName_);
                // Client's file hash
                auto clientFileHash = std::string(packet_->data.data(), packet_->header.length);
                if (hash != clientFileHash) {
                    spdlog::error("Client file hash and our hash is different: {} vs {}", clientFileHash, hash);
                } else {
                    spdlog::info("Files hashes are same.");
                }
                spdlog::info("Packet::Type::Hash: {}", packet_->header.length);
                break;
            };
            case (Header::Type::Exit): {
                std::fclose(file_);
                spdlog::info("Packet::Type::Exit");
                break;
            };
            default: {
                spdlog::error("Unknown packet");
                break;
            }
        }
    }

    void notifyComplete() {
        packetFlow_->setPacketForNextStage(PacketFlow<T>::Stage::Socket, std::move(packet_));
        std::cout << "notifyComplete()" << std::endl;
    }



    bool isFileExist(const string& fileName) {
        return std::filesystem::exists(fileName);
    }

    std::string getUniqueName(const string& fileName) {
        std::string uniqueName;
        auto ts = to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

        auto dotPos = fileName.find('.');

        if (dotPos != std::string::npos) {
            uniqueName = fileName.substr(0, dotPos) + ts + fileName.substr(dotPos, fileName.size());
        } else {
            uniqueName = fileName + ts;
        }

        return uniqueName;
    }

    std::string getFileHash(const std::string& filename) {
        std::ifstream file(filename, std::ifstream::binary);

        if (!file) {
            std::cerr << "File " << filename << " cannot be opened." << std::endl;
            return "";
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];

        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        const size_t bufferSize = 1 << 12; // 4096 bytes
        char buffer[bufferSize];
        while (file.good()) {
            file.read(buffer, bufferSize);
            SHA256_Update(&sha256, buffer, file.gcount());
        }

        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }

        return ss.str();
    }

protected:
    std::FILE* file_;
    std::shared_ptr<PacketFlow<T>> packetFlow_;
    std::unique_ptr<T> packet_;
    std::string fileName_;
};

TEST(test_file, test_writer) {
    std::shared_ptr<PacketFlow<MyPacket<char>>> packetFlow = std::make_shared<PacketFlow<MyPacket<char>>>();

    std::jthread([&packetFlow]()
    {
         // set file name
         {
             std::unique_ptr<MyPacket<char>> packet = packetFlow->getPacket(PacketFlow<MyPacket<char>>::Stage::Socket);
             packet->header.type = Header::Type::FileName;
             packet->header.length = 7;
             packet->data = {'t', 'm', 'p', '.', 't', 'x', 't'};
             packetFlow->setPacketForNextStage(PacketFlow<MyPacket<char>>::Stage::File, std::move(packet));
         }
         // set some data
         {
             std::unique_ptr<MyPacket<char>> packet = packetFlow->getPacket(PacketFlow<MyPacket<char>>::Stage::Socket);
             packet->header.type = Header::Type::FileData;
             packet->header.length = 3;
             packet->data = {'k', 'e', 'k'};
             packetFlow->setPacketForNextStage(PacketFlow<MyPacket<char>>::Stage::File, std::move(packet));
         }
         // set close a file
         {
             std::unique_ptr<MyPacket<char>> packet = packetFlow->getPacket(PacketFlow<MyPacket<char>>::Stage::Socket);
             packet->header.type = Header::Type::Exit;
             packetFlow->setPacketForNextStage(PacketFlow<MyPacket<char>>::Stage::File, std::move(packet));
         }
    });

    std::jthread([&packetFlow]()
    {
         FileWriter<MyPacket<char>> fileWriter(packetFlow);
         for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
             fileWriter.waitNextData();
             fileWriter.processData();
             fileWriter.notifyComplete();
         }
    });
}

class MyServer {

};

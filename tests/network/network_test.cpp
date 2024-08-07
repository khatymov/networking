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
    PacketFlow(const PacketFlow&) = delete;
    PacketFlow(PacketFlow&&) = delete;
    PacketFlow operator=(const PacketFlow&) = delete;
    PacketFlow operator=(PacketFlow&&) = delete;
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
//        spdlog::info("Kakoi deadlock nahui");
        std::unique_ptr<PacketType> packet;
        auto* currentQueue = getQueueForStage(stage);
        {
            std::lock_guard<std::mutex> lockGuard(mutex_);
            if (not currentQueue->empty()) {
                packet = std::move(currentQueue->front());
                currentQueue->pop();
            }
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

TEST(test_packet_rotator, test_default_packet) {
    std::shared_ptr<PacketFlow<>> packetFlow = std::make_shared<PacketFlow<>>();
}

TEST(test_packet_rotator, test_crypto_packet_constructor) {
    std::shared_ptr<PacketFlow<MyPacket<CryptoPP::byte>>> packetFlow = std::make_shared<PacketFlow<MyPacket<CryptoPP::byte>>>();
}

TEST(test_packet_rotator, test_getPacket) {
    auto packetFlow = std::make_shared<PacketFlow<>>();

    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        auto packet = packetFlow->getPacket(PacketFlow<>::Stage::Socket);
        EXPECT_TRUE(packet != nullptr);
    }

    EXPECT_EQ(packetFlow->getPacket(PacketFlow<MyPacket<char>>::Stage::Socket), nullptr);
}

TEST(test_packet_rotator, test_setPacketForNextStage) {
    auto packetFlow = std::make_shared<PacketFlow<>>();

    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        auto packet = packetFlow->getPacket(PacketFlow<>::Stage::Socket);
        packet->header.type = Header::Type::FileData;
        packetFlow->setPacketForNextStage(PacketFlow<MyPacket<char>>::Stage::File, std::move(packet));
    }

    EXPECT_EQ(packetFlow->getPacketsAmount(), packetFlow->getPacketsAmountInQueues());

    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        auto packet = packetFlow->getPacket(PacketFlow<>::Stage::File);
        EXPECT_EQ(packet->header.type, Header::Type::FileData);
    }

    EXPECT_EQ(packetFlow->getPacket(PacketFlow<>::Stage::File), nullptr);
}


TEST(test_packet_rotator, test_allStages) {
    std::shared_ptr<PacketFlow<MyPacket<CryptoPP::byte>>> packetFlow = std::make_shared<PacketFlow<MyPacket<CryptoPP::byte>>>();

    //SOCKET -> CRYPTO
    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        auto packet = packetFlow->getPacket(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::Socket);
        packet->header.type = Header::Type::CryptoData;
        packet->data = {static_cast<CryptoPP::byte>(i)};
        packetFlow->setPacketForNextStage(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::Crypto, std::move(packet));
    }

    EXPECT_EQ(packetFlow->getPacketsAmount(), packetFlow->getPacketsAmountInQueues());

    //CRYPTO -> FILE
    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        auto packet = packetFlow->getPacket(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::Crypto);
        packet->header.type = Header::Type::FileData;
        packetFlow->setPacketForNextStage(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::File, std::move(packet));
    }

    EXPECT_EQ(packetFlow->getPacketsAmount(), packetFlow->getPacketsAmountInQueues());

    //FILE -> SOCKET
    for (int i = 0; i < packetFlow->getPacketsAmount(); i++) {
        auto packet = packetFlow->getPacket(PacketFlow<MyPacket<CryptoPP::byte>>::Stage::File);
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
        (static_cast<Derived*>(this)->waitNextDataImpl());
    }
    void processData() {
        (static_cast<Derived*>(this)->processDataImpl());
    }
    void notifyComplete() {
        (static_cast<Derived*>(this)->notifyCompleteImpl());
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

    void waitNextDataImpl() {
        std::cout << "FileWriter void waitNextDataImpl() " << std::endl;
        while (!(packet_ = packetFlow_->getPacket(PacketFlow<T>::Stage::File))) {
            std::this_thread::yield();
//            std::cout << "FileWriter waitNextData() queue File is empty" << std::endl;
            continue;
        }
        std::cout << "FileWriter waitNextData() got the packet" << std::endl;
    }

    void processDataImpl() {
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

    void notifyCompleteImpl() {
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
    auto packetFlow = std::make_shared<PacketFlow<>>();

    std::jthread([&packetFlow]()
    {
         // set file name
         {
             auto packet = packetFlow->getPacket(PacketFlow<>::Stage::Socket);
             packet->header.type = Header::Type::FileName;
             packet->header.length = 7;
             packet->data = {'t', 'm', 'p', '.', 't', 'x', 't'};
             packetFlow->setPacketForNextStage(PacketFlow<>::Stage::File, std::move(packet));
         }
         // set some data
         {
             auto packet = packetFlow->getPacket(PacketFlow<>::Stage::Socket);
             packet->header.type = Header::Type::FileData;
             packet->header.length = 3;
             packet->data = {'k', 'e', 'k'};
             packetFlow->setPacketForNextStage(PacketFlow<>::Stage::File, std::move(packet));
         }
         // set close a file
         {
             auto packet = packetFlow->getPacket(PacketFlow<>::Stage::Socket);
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

using namespace boost::asio;
using namespace boost::asio::ip;

template<typename T = MyPacket<char>>
class MyConnection: public IStreamProcessor<T>, public std::enable_shared_from_this<MyConnection<T>>{
public:
    MyConnection(boost::asio::ip::tcp::socket socket, std::shared_ptr<PacketFlow<T>> packetFlow)
        :socket_(std::move(socket)),
         packetFlow_(packetFlow)
    {
        cout << "PTR COUNTER in MyConnection(): " << packetFlow_.use_count() << endl;
    }

    ~MyConnection() {
        cout << "~MyConnection(): " << endl;
    }

    void run() {
        waitNextDataImpl();
    }

    // wait for packet from PacketFlow
    // will be used for a recursive call
    void waitNextDataImpl() {
        spdlog::info("Connection waitNextDataImpl");
        cout << "PTR COUNTER in waitNextDataImpl: " << packetFlow_.use_count() << endl;
        while (!(packet_ = packetFlow_->getPacket(PacketFlow<T>::Stage::Socket))) {
            std::this_thread::yield();
//            std::cout << "waitNextData() queue Socket is empty" << std::endl;
            continue;
        }
        processDataImpl();
    }

    // read from socket to the gotten packet
    // send ack
    void processDataImpl() {
        spdlog::info("Connection processDataImpl()");
        readHeader_();
    }

    // transfer filled packet to PacketFlow back
    void notifyCompleteImpl() {
        //TODO: fix stage to Crypto!!
        spdlog::info("Connection notifyCompleteImpl()");
        packetFlow_->setPacketForNextStage(PacketFlow<T>::Stage::File, std::move(packet_));
        std::cout << "notifyComplete() Connection" << std::endl;
        waitNextDataImpl();
    }

protected:

    void readHeader_() {
        auto self = this->shared_from_this();
        boost::asio::async_read(socket_, boost::asio::buffer(&packet_->header, sizeof(packet_->header)),
                                [this, self] (boost::system::error_code errorCode, std::size_t /*length*/)
                                {
                                    spdlog::info("Connection got header");
                                    if (!errorCode) {
                                        if (packet_->header.length > 0) {
                                            spdlog::info("Connection read data. Data > 0 ");
                                            readPayload_();
                                        } else {
                                            spdlog::info("Connection send Ack. Data = 0 ");
                                            T packetAck;
                                            packetAck.header.type = Header::Type::Ack;
                                            writeHeader_(packetAck);
                                        }
                                    } else {
                                        spdlog::error("Read header error: {}", errorCode.message());
                                        socket_.close();
                                    }
                                });
    }

    void readPayload_() {
        auto self = this->shared_from_this();
        boost::asio::async_read(socket_, boost::asio::buffer(packet_->data, packet_->header.length),
                                [this, self] (boost::system::error_code errorCode, std::size_t /*length*/)
                                {
                                    if (errorCode) {
                                        spdlog::error("Read header error: {}", errorCode.message());
                                        socket_.close();
                                    } else {
                                        std::string str(packet_->data.data(), packet_->header.length);
                                        spdlog::info("Connection got data: {}", str);
                                        T packetAck;
                                        packetAck.header.type = Header::Type::Ack;
                                        packetAck.header.length = 0;
                                        writeHeader_(packetAck);
                                    }
                                });
    }

    void writeHeader_(const T& packet) {
        auto self = this->shared_from_this();
        boost::asio::async_write(socket_, boost::asio::buffer(&packet.header, sizeof(packet.header)),
                                 [this, self, packet](boost::system::error_code errorCode, std::size_t /*length*/) {
                                     if (!errorCode) {
                                         writePayload_(packet);
                                     } else {
                                         spdlog::error("Send packet header error: {}", errorCode.message());
                                         socket_.close();
                                     }
                                 });
    }

    void writePayload_(const T& packet) {
        auto self = this->shared_from_this();
        boost::asio::async_write(socket_, boost::asio::buffer(&packet.data, packet.header.length),
                                 [this, self](boost::system::error_code errorCode, std::size_t /*length*/) {
                                     if (errorCode) {
                                         spdlog::error("Send packet payload error: {}", errorCode.message());
                                         socket_.close();
                                     }
                                     notifyCompleteImpl();
                                 });
    }

    boost::asio::ip::tcp::socket socket_;
    std::shared_ptr<PacketFlow<T>> packetFlow_;
    std::unique_ptr<T> packet_;
};



class MyServer {
public:
    MyServer(boost::asio::io_context& io_context, const std::string& ip = "127.0.0.1", const uint port = 1234)
        : acceptor_(io_context, ip::tcp::endpoint(boost::asio::ip::make_address(ip), port)) {
        spdlog::info("Server started with ip and port: [{}:{}]", ip, port);
    }

    template <typename T = MyPacket<char>>
    void run(std::shared_ptr<PacketFlow<T>> packetFlow) {
        cout << "PTR COUNTER in run(): " << packetFlow.use_count() << endl;
        waitNewConnection_<T>(packetFlow);
    }

protected:
    template <typename T = MyPacket<char>>
    void waitNewConnection_(std::shared_ptr<PacketFlow<T>> packetFlow) {
        acceptor_.async_accept([this, packetFlow] (boost::system::error_code errorCode, tcp::socket socket)
        {
            if (!errorCode) {
                spdlog::info("New connection.");
                cout << "PTR COUNTER in acceptor_.async_accept: " << packetFlow.use_count() << endl;
                std::make_shared<MyConnection<T>>(std::move(socket), packetFlow)->run();
            } else {
                spdlog::error("New connection error: {}", errorCode.message());
            }
            // Prime the asio context with more work - again simply wait for
            // another connection...
            waitNewConnection_<T>(packetFlow);
        });

    }
    tcp::acceptor acceptor_;
};

namespace SocketIO {
    template <typename T>
    [[nodiscard]] static bool writeToSocket(ip::tcp::socket& socket, const MyPacket<T>& packet, boost::system::error_code& ec) {
        //send header
        auto sentHeaderSize = write(socket, boost::asio::buffer(&packet.header, sizeof(packet.header)), transfer_exactly(sizeof(packet.header)), ec);
        if (ec)
        {
            spdlog::error("Send packet header error: {}", ec.message());
            return false;
        }
        //send payload
        if (packet.header.length > 0) {
            auto sentPayloadSize = write(socket, boost::asio::buffer(&packet.data, packet.header.length), transfer_exactly(packet.header.length), ec);
            if (ec) {
                spdlog::error("Send packet payload error: {}", ec.message());
                return false;
            }
        }

        return true;
    }
    template <typename T>
    [[nodiscard]] static bool readFromSocket(ip::tcp::socket& socket, MyPacket<T>& packet, boost::system::error_code& ec) {
        //read header
        auto head_size = read(socket, buffer(&packet.header, sizeof(packet.header)), transfer_exactly(sizeof(packet.header)), ec);
        if (ec)
        {
            spdlog::error("Read packet header error: {}", ec.message());
            return false;
        }
        //read payload
        auto payload_size = read(socket, boost::asio::buffer(&packet.data, packet.header.length), transfer_exactly(packet.header.length), ec);
        if (ec)
        {
            spdlog::error("Read packet payload error: {}", ec.message());
            return false;
        }

        return true;
    }
}


class MyClient {
    MyClient(const MyClient&) = delete;
    MyClient(MyClient&&) = delete;
    MyClient operator=(const MyClient&) = delete;
    MyClient operator=(MyClient&&) = delete;
public:
    MyClient(const std::string& ip = "127.0.0.1", const uint port = 1234)
    : endpoint_{address::from_string(ip), static_cast<port_type>(port)}
    , socket_{context_} {}

    [[maybe_unused]] bool connect() {
        boost::system::error_code errorCode;
        socket_.connect(endpoint_, errorCode);

        if (errorCode) {
            spdlog::error("failed to connect, error: {}", errorCode.message());
            return false;
        }

        spdlog::info("Client connected");
        return true;
    }

    bool sendFile(const std::string& fileName) {
        // 4 stages:
        // 1 - send file name
        // 2 - send data from file to socket
        // 3 - send hash of a file
        // 4 - send exit and finish
        MyPacket<> packet;
        boost::system::error_code ec;
        // stage 1 - send file name
        {
            packet.header.type = Header::Type::FileName;
            packet.header.length = fileName.size();
            memcpy(packet.data.data(), fileName.c_str(), fileName.size());

            if (!SocketIO::writeToSocket(socket_, packet, ec)) {
                return false;
            }

            if (!SocketIO::readFromSocket(socket_, packet, ec))  {
                spdlog::error("Didn't get ack packet for FileData packet");
            }
        }
        // stage 2 - send data from file to socket
        {
            std::FILE* m_file = std::fopen(fileName.c_str(), "r");
            if (m_file == nullptr) {
                throw std::runtime_error("Can't open a file" + fileName);
            }
            MyPacket<char> packet;
            do {
                packet.header.length = std::fread(&packet.data, sizeof(char), DATA_SIZE, m_file);
                packet.header.type = Header::Type::FileData;
                const bool everything_done = packet.header.length == 0;

                if (everything_done)
                {
                    break;
                }

                if (!SocketIO::writeToSocket(socket_, packet, ec)) {
                    return false;
                }

                if (!SocketIO::readFromSocket(socket_, packet, ec))  {
                    spdlog::error("Didn't get ack packet for FileData packet");
                }
            } while (true);
        }

        // stage 4 - send exit and finish
        {
            packet.header.type = Header::Type::Exit;
            packet.header.length = 1;

            if (!SocketIO::writeToSocket(socket_, packet, ec)) {
                return false;
            }

            if (!SocketIO::readFromSocket(socket_, packet, ec)) {
                spdlog::error("Didn't get ack packet for Exit packet");
            }

            spdlog::debug("Exit packet was sent. Finish");
        }
    }

    //! \brief asio context handles the data transfer...
    boost::asio::io_context context_;
    //! \brief using to resolve hostname/ip-address into tangiable physical address
    boost::asio::ip::tcp::endpoint endpoint_;
    //! \brief socket allows us to connect to the server
    boost::asio::ip::tcp::socket socket_;
};

TEST(test_connection, test_costructor) {
    std::chrono::seconds duration(2);
    auto packetFlow = std::make_shared<PacketFlow<>>();
    boost::asio::io_context io_context;
    MyServer myServer(io_context);
    myServer.run(packetFlow);

    // Number of threads you want to run io_context in.
    const std::size_t num_threads = 1;// std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    for(std::size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&io_context, &duration]() {
//            io_context.run();
            io_context.run_for(duration);
        });
    }


    std::thread fileThread([packetFlow, &duration]()
    {
        auto start = std::chrono::high_resolution_clock::now();
        FileWriter<MyPacket<char>> fileWriter(packetFlow);
        while(true) {
//            auto now = std::chrono::high_resolution_clock::now();
//            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start);
//            // Check if the elapsed time is greater than or equal to the duration
//            if (elapsed >= duration) {
//                break;
//            }
            fileWriter.waitNextData();
            fileWriter.processData();
            fileWriter.notifyComplete();
        }
    });

    std::jthread([]()
    {
        spdlog::default_logger()->set_pattern("[Hey] %+ [thread %t]");
        spdlog::info("Client starts work");
        MyClient client;
        if (client.connect()) {
            std::string file =
                std::string(PATH_TO_ASSETS) + std::string("textClient1.txt");
            client.sendFile(file);
        } else {
            spdlog::info("Client could not connect to a server");
        }
    });

    std::jthread([]()
    {
        MyClient client;
        if (client.connect()) {
            std::string file =
                std::string(PATH_TO_ASSETS) + std::string("textClient2.txt");
            client.sendFile(file);
        } else {
            spdlog::info("Client could not connect to a server");
        }
    });

    fileThread.join();
    for(auto& thrd : threads) {
        if(thrd.joinable()) {
            thrd.join();
        }
    }
}

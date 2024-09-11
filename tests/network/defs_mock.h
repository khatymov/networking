#pragma once

#include "encryptor.h"
#include "decryptor.h"
#include "connection.h"
#include "gtest/gtest.h"
#include "my_packet.h"
#include "thread_safe_queue.h"

using namespace std;
using namespace testing;
using namespace network;

using namespace boost::asio;
using namespace boost::asio::ip;

template <typename T = MyPacket<char>>
std::vector<std::shared_ptr<ThreadSafeQueue<T>>> get2Queue() {
    return {std::make_shared<ThreadSafeQueue<T>>(true),
            std::make_shared<ThreadSafeQueue<T>>(false)};
}

template <typename T>
std::vector<std::shared_ptr<ThreadSafeQueue<T>>> get3Queue() {
    return {std::make_shared<ThreadSafeQueue<T>>(true),
            std::make_shared<ThreadSafeQueue<T>>(false),
            std::make_shared<ThreadSafeQueue<T>>(false)};
}

extern std::string gHelloWorldStr;

constexpr std::string ipDefualt("127.0.0.1");
constexpr uint portDefualt = 1234;

// CRYPTO
template <typename DataType>
class EncryptorMock: public Encryptor<DataType> {
public:
    EncryptorMock(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                  std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
        : Encryptor<DataType>(currentQueue, nextQueue) {
        this->data_ = std::make_unique<DataType>();
        this->data_->header.type = Header::Type::FileData;

        this->data_->header.length = gHelloWorldStr.size();
        std::copy(gHelloWorldStr.begin(), gHelloWorldStr.end(), this->data_->data.begin());
    }

    void setData(std::unique_ptr<DataType>&& data) {
        this->data_ = std::move(data);
    }

    std::unique_ptr<DataType> getData() {
        return std::move(this->data_);
    }
};

template <typename DataType>
class PacketGeneratorMock: public DataProcessor<PacketGeneratorMock<DataType>, DataType> {
public:
    PacketGeneratorMock(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                        std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
        : DataProcessor<PacketGeneratorMock<DataType>, DataType>(currentQueue, nextQueue) {}

    void set(std::unique_ptr<DataType>&& packet) {
        this->data_ = std::move(packet);
    }
};


template <typename DataType>
class PacketCollectorMock: public DataProcessor<PacketCollectorMock<DataType>, DataType> {
public:
    PacketCollectorMock(std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                        std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
        : DataProcessor<PacketCollectorMock<DataType>, DataType>(currentQueue, nextQueue) {}

    void collect() {
        if (this->data_->header.type == Header::Type::FileData)
            dataVec.push_back(std::make_unique<DataType>(DataType{this->data_->header, this->data_->data}));
        if (this->data_->header.type == Header::Type::Exit) {
            this->isProcessDone_ = true;
        }
    }

    std::vector<unique_ptr<DataType>> dataVec;
};

// Connection
template <typename T>
inline std::unique_ptr<T> getPacketWithFileData(char ch) {
    std::unique_ptr<T> packetWithFileData = std::make_unique<T>();

    packetWithFileData->header.type = Header::Type::FileData;
    packetWithFileData->header.length = 1;
    packetWithFileData->data[0] = ch;

    return std::move(packetWithFileData);
};
template <typename T>
std::vector<std::unique_ptr<T>> getAlphabetPacks() {
    std::vector<std::unique_ptr<T>> res;
    for (char letter = 'a'; letter <= 'z'; letter++) {
        res.push_back(getPacketWithFileData<T>(letter));
    }
    return res;
}

// 4. test processDataImpl()
// a. read test
// b. write test
// c. read/write

// finish DummyServer

// create a client
// separate thread and function for the client

// The client should attach to the endpoint, create socket and send some package to a server, as simple as it is
template <typename T>
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
        auto packetWithFileData = getPacketWithFileData<MyPacket<char>>('a');
        EXPECT_TRUE(Connection<T>::write(socket_, packetWithFileData, errorCode));
        EXPECT_TRUE(Connection<T>::read(socket_, packetWithFileData, errorCode));
        EXPECT_EQ(packetWithFileData->header.type, Connection<T>::ackPacket.header.type);
    }

    //! \brief asio context handles the data transfer...
    boost::asio::io_context context_;
    //! \brief using to resolve hostname/ip-address into tangiable physical address
    boost::asio::ip::tcp::endpoint endpoint_;
    //! \brief socket allows us to connect to the server
    boost::asio::ip::tcp::socket socket_;
};

template <typename T>
class ConnectionMock : public Connection<T> {
public:
    ConnectionMock(const Mode mode, boost::asio::ip::tcp::socket socket, std::shared_ptr<ThreadSafeQueue<T>> currentQueue,
                   std::shared_ptr<ThreadSafeQueue<T>> nextQueue)
        : Connection<T>(mode, std::move(socket), currentQueue, nextQueue){};

    void setData(std::unique_ptr<T>&& data) { this->data_ = std::move(data); }

    std::unique_ptr<T> getData() { return std::move(this->data_); }
};

template <typename T>
struct DummyServer {
    DummyServer(const std::string& ip = ipDefualt, const uint port = portDefualt)
        : acceptor_(io_context_, ip::tcp::endpoint(boost::asio::ip::make_address(ip), port)) {}

    void start() {
        tcp::socket socket(io_context_);
        acceptor_.accept(socket);
        // won't be used
        auto tsQueues = get2Queue<MyPacket<char>>();
        ConnectionMock<MyPacket<char>> connectionMock(Mode::Server, std::move(socket), tsQueues[0], tsQueues[1]);
        // allocate memory for packet
        connectionMock.setData(std::move(make_unique<MyPacket<char>>()));
        // connection should accept data from socket
        connectionMock.processData();
        // get result for verification
        auto resPacket = connectionMock.getData();
        auto expectedPacket = getPacketWithFileData<MyPacket<char>>('a');

        EXPECT_EQ(resPacket->header.type, expectedPacket->header.type);
        EXPECT_EQ(resPacket->header.length, expectedPacket->header.length);
        EXPECT_EQ(resPacket->data[0], expectedPacket->data[0]);
    }

    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
};

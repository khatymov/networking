#include "gtest/gtest.h"

#include "connection_test.h"
#include "packet.h"
#include "socket_messenger/socket_messenger.h"

using namespace std;
using namespace testing;
using namespace boost::asio;
using namespace boost::asio::ip;


int port = 1234;
std::string host("127.0.0.1");

class SimpleClient {
public:
    SimpleClient(const std::string ip, const unsigned short port)
        : m_endpoint{address::from_string(ip), port}
        , m_socket{m_context}
    {
    }

    ~SimpleClient() = default;

public:

    bool connect() {
        boost::system::error_code ec;
        m_socket.connect(m_endpoint, ec);
        if (ec)
        {
            spdlog::error("failed to connect, error: {}", ec.message());
            return false;
        }

        spdlog::info("Client connected");

        return true;
    }

    bool sendWelcomePacket(char data[4096]) {
        Packet packet;
        memcpy(packet.payload, data, DATA_SIZE);
        boost::system::error_code ec;
        write(m_socket, boost::asio::buffer(&packet.payload, sizeof(packet.payload)), transfer_exactly(sizeof(packet.payload)), ec);
        if (ec)
        {
            spdlog::error("Client send packet error: {}", ec.message());
            return false;
        }

        return true;
    }

    bool isPongable() {

        //send ping packet
        Packet packet;
        packet.header.type = Packet::Type::Exit;
        packet.header.length = 0;
        boost::system::error_code ec;
        spdlog::debug("Client, wait ping");
        //wait for ping packet
        if (!readFromSocket(m_socket, packet, ec)) {
            return false;
        }

        if (packet.header.type == Packet::Type::Ping)
        {
            spdlog::debug("Client got Packet::Ping with size {}", packet.header.length);
        }

        spdlog::debug("Client, send pong");
        packet.header.type = Packet::Type::Pong;
        if (!writeToSocket(m_socket, packet, ec)) {
            return false;
        }

        return packet.header.type == Packet::Type::Pong;
    }

    bool sendFileNamePacket() {
        Packet packet;
        packet.header.type = Packet::Type::FileName;
        std::string name("tmp.txt");
        packet.header.length = name.size();
        memcpy(packet.payload, name.c_str(), name.size());
        boost::system::error_code ec;
        if (!writeToSocket(m_socket, packet, ec)) {
            return false;
        }

        return true;
    }

    bool fullCycle(const std::string& fileName) {
        FileHandler fileHandler;
        if (fileHandler.isFileExist(fileName)) {
            spdlog::error("File doesn't exist");
//            return false;
        }

        fileHandler.open(fileName, "rb");

        Packet packet;
        packet.header.type = Packet::Type::FileName;
        packet.header.length = fileName.size();
        memcpy(packet.payload, fileName.c_str(), fileName.size());
        boost::system::error_code ec;
        if (!writeToSocket(m_socket, packet, ec)) {
            return false;
        }

        if (!readFromSocket(m_socket, packet, ec)) {
            spdlog::debug("Didn't get ack pack Packet::Type::FileName {}", packet.header.length);
        } else if (packet.header.type == Packet::Type::Ack) {
            spdlog::debug("ACK PACK Packet::Type::FileName {}", packet.header.length);
        }

        do {
            fileHandler.read(packet);
            packet.header.type = Packet::Type::FileData;
            const bool everything_done = packet.header.length == 0;

            if (everything_done)
            {
                break;
            }

            if (!writeToSocket(m_socket, packet, ec)) {
                return false;
            }

            if (!readFromSocket(m_socket, packet, ec)) {
                spdlog::debug("Didn't get ack pack Packet::Type::FileData {}", packet.header.length);
            } else if (packet.header.type == Packet::Type::Ack) {
                spdlog::debug("ACK PACK Packet::Type::FileData {}", packet.header.length);
            }

        } while (true);

        packet.header.type = Packet::Type::Hash;
        auto hash = fileHandler.getFileHash(fileName);
        memcpy(packet.payload, hash.c_str(), hash.size());
        packet.header.length = hash.size();
        if (!writeToSocket(m_socket, packet, ec)) {
            return false;
        }

        if (!readFromSocket(m_socket, packet, ec)) {
            spdlog::debug("Didn't get ack pack  Packet::Type::Hash {}", packet.header.length);
        } else if (packet.header.type == Packet::Type::Ack) {
            spdlog::debug("ACK PACK Packet::Type::Hash  {}", packet.header.length);
        }

        packet.header.type = Packet::Type::Exit;
        packet.header.length = 0;

        if (!writeToSocket(m_socket, packet, ec)) {
            return false;
        }

        return true;
    }

private:
    // asio context handles the data transfer...
    io_context m_context;
    tcp::endpoint m_endpoint;
    tcp::socket m_socket;

};

bool doClientConnection() {
    SimpleClient simpleClient(host, port);
    return simpleClient.connect();
}

bool sendPacketFromClient(char data[4096]) {
    SimpleClient simpleClient(host, port);
    return simpleClient.connect() and simpleClient.sendWelcomePacket(data);
}

bool checkPingPong() {
    SimpleClient simpleClient(host, port);
    return simpleClient.connect() and simpleClient.isPongable();
}

bool sendFileNamePacket() {
    SimpleClient simpleClient(host, port);
    return simpleClient.connect() and simpleClient.isPongable() and simpleClient.sendFileNamePacket();
}

bool sendFilePacket(string filename) {
    SimpleClient simpleClient(host, port);
    return simpleClient.connect() and simpleClient.isPongable() and simpleClient.fullCycle(filename);
}

TEST(test_connection, test_connection) {
    io_context ioContext;
    tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    tcp::acceptor myAcceptor(ioContext, endpoint);
    myAcceptor.async_accept([&] (boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            spdlog::info("New connection");
            std::shared_ptr<ConnectionInterface> newConnection = std::make_shared<ConnectionInterface>(ioContext, std::move(socket));
            EXPECT_TRUE(newConnection->ConnectToClient());
        } else {
            spdlog::error("Error has occurred during acceptance: {}", ec.message());
        }
    });

    EXPECT_TRUE(doClientConnection());

    ioContext.run();
}

//Get a message and print it
TEST(test_connection, test_process) {
    io_context ioContext;
    tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    tcp::acceptor myAcceptor(ioContext, endpoint);
    myAcceptor.async_accept([&] (boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            spdlog::info("New connection");
            std::shared_ptr<ConnectionInterface> newConnection = std::make_shared<ConnectionInterface>(ioContext, std::move(socket));
            if (newConnection->ConnectToClient()) {
                newConnection->Process();
            }
        } else {
            spdlog::error("Error has occurred during acceptance: {}", ec.message());
        }
    });
    char data[DATA_SIZE] = "hello, I'm a Client";
    sendPacketFromClient(data);

    ioContext.run();
}

TEST(test_file_writer_connection, test_connection) {
    io_context ioContext;
    tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    tcp::acceptor myAcceptor(ioContext, endpoint);
    myAcceptor.async_accept([&] (boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            spdlog::info("New connection");
            std::shared_ptr<FileWriterConnection> newConnection = std::make_shared<FileWriterConnection>(ioContext, std::move(socket));
            EXPECT_TRUE(newConnection->ConnectToClient());
        } else {
            spdlog::error("Error has occurred during acceptance: {}", ec.message());
        }
    });

    EXPECT_TRUE(doClientConnection());

    ioContext.run();
}


TEST(test_file_writer_connection, test_process) {
    io_context ioContext;
    tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    tcp::acceptor myAcceptor(ioContext, endpoint);
    myAcceptor.async_accept([&] (boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            spdlog::info("New connection");
            std::shared_ptr<FileWriterConnection> newConnection = std::make_shared<FileWriterConnection>(ioContext, std::move(socket));
            if (newConnection->ConnectToClient()) {
                newConnection->Process();
            }
        } else {
            spdlog::error("Error has occurred during acceptance: {}", ec.message());
        }
    });
    char data[DATA_SIZE] = "hello, I'm a Client";
    sendPacketFromClient(data);

    ioContext.run();
}

//Get a message and print it
TEST(test_file_writer_connection, test_ping_pong) {
    io_context ioContext;
    tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    tcp::acceptor myAcceptor(ioContext, endpoint);
    myAcceptor.async_accept([&] (boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            spdlog::info("New connection");
            std::shared_ptr<FileWriterConnection> newConnection = std::make_shared<FileWriterConnection>(ioContext, std::move(socket));
            if (newConnection->ConnectToClient()) {
                EXPECT_TRUE(newConnection->isPingable());
            }
        } else {
            spdlog::error("Error has occurred during acceptance: {}", ec.message());
        }
    });

    auto future = std::async(std::launch::async, checkPingPong);

    ioContext.run();
    auto isPongable = future.get();
    EXPECT_TRUE(isPongable);
}

//Get a message and print it
TEST(test_file_writer_connection, test_ping_pong_unique) {
    io_context ioContext;
    tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    tcp::acceptor myAcceptor(ioContext, endpoint);
    myAcceptor.async_accept([&] (boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            spdlog::info("New connection");
            std::unique_ptr<FileWriterConnection> newConnection = std::make_unique<FileWriterConnection>(ioContext, std::move(socket));
            if (newConnection->ConnectToClient()) {
                EXPECT_TRUE(newConnection->isPingable());
            }
        } else {
            spdlog::error("Error has occurred during acceptance: {}", ec.message());
        }
    });

    auto future = std::async(std::launch::async, checkPingPong);

    ioContext.run();
    auto isPongable = future.get();
    EXPECT_TRUE(isPongable);
}

//Get a message and print it
TEST(test_file_writer_connection, test_file) {
    io_context ioContext;
    tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    tcp::acceptor myAcceptor(ioContext, endpoint);
    myAcceptor.async_accept([&] (boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            spdlog::info("New connection");
            std::shared_ptr<FileWriterConnection> newConnection = std::make_shared<FileWriterConnection>(ioContext, std::move(socket));
            if (newConnection->ConnectToClient()) {
                newConnection->FileProcess();
            }
        } else {
            spdlog::error("Error has occurred during acceptance: {}", ec.message());
        }
    });

    auto future = std::async(std::launch::async, sendFileNamePacket);

    ioContext.run();
    auto isPacketSent = future.get();
    EXPECT_TRUE(isPacketSent);
}

//Get a message and print it
TEST(test_file_writer_connection, test_send_file) {
    io_context ioContext;
    tcp::endpoint endpoint(boost::asio::ip::make_address(host), port);

    tcp::acceptor myAcceptor(ioContext, endpoint);
    myAcceptor.async_accept([&] (boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            spdlog::info("New connection");
            std::shared_ptr<FileWriterConnection> newConnection = std::make_shared<FileWriterConnection>(ioContext, std::move(socket));
            if (newConnection->ConnectToClient()) {
                newConnection->FileProcess();
            }
        } else {
            spdlog::error("Error has occurred during acceptance: {}", ec.message());
        }
    });

    auto future = std::async(std::launch::async, sendFilePacket, "Somedata.txt");

    ioContext.run();
    auto isPacketSent = future.get();
    EXPECT_TRUE(isPacketSent);
}
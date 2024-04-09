/*! \file client.cpp
 * \brief Client class implementation.
 */


#include "client.h"
#include "packet.h"
#include "socket_messenger/socket_messenger.h"
#include "file_handler.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

Client::Client(const std::string& ip, const uint port)
    : m_endpoint{address::from_string(ip), static_cast<port_type>(port)}
    , m_socket{m_context} {

}
[[maybe_unused]] bool Client::connect() {
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
bool Client::doPingPing() {
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
bool Client::sendFile(const string& fileName) {
    FileHandler fileHandler;
//    if (fileHandler.isFileExist(fileName)) {
//        spdlog::error("File doesn't exist");
//        return false;
//    }

    fileHandler.open(fileName, "rb");

    Packet packet;
    packet.header.type = Packet::Type::FileName;
    packet.header.length = fileName.size();
    memcpy(packet.payload, fileName.c_str(), fileName.size());
    boost::system::error_code ec;
//    send(packet);
    if (!writeToSocket(m_socket, packet, ec)) {
        return false;
    }

//    this_thread::sleep_for(std::chrono::seconds(35));
//    if (!readFromSocket(m_socket, packet, ec)) {
//        spdlog::debug("Didn't get ack pack Packet::Type::FileName {}", packet.header.length);
//    } else if (packet.header.type == Packet::Type::Ack) {
//        spdlog::debug("ACK PACK Packet::Type::FileName {}", packet.header.length);
//    }

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
    packet.header.length = 1;

    if (!writeToSocket(m_socket, packet, ec)) {
        return false;
    }

    if (!readFromSocket(m_socket, packet, ec)) {
        spdlog::debug("Didn't get ack pack  Packet::Type::Exit {}", packet.header.length);
    } else if (packet.header.type == Packet::Type::Ack) {
        spdlog::debug("Got Packet::Type::Exit  {}", packet.header.length);
    }

    spdlog::debug("SEND EXIT PACK");
    return true;
}
void Client::send(const Packet& packet) {
    post(m_context,
         [this, packet]() {
             writeHeader(packet);
         });
}
void Client::writeHeader(const Packet& packet) {
    async_write(m_socket, buffer(&packet.header, sizeof(packet.header)),
                [this, packet](boost::system::error_code ec, size_t length) {
                   if (!ec) {
                       if (packet.header.length > 0) {
                           writePayload(packet);
                       }
                   } else {
                       spdlog::error("Send packet header error: {}. Close socket.", ec.message());
                       m_socket.close();
                   }
                });
}
void Client::writePayload(const Packet& packet) {
    async_write(m_socket, buffer(&packet.payload, packet.header.length),
                [this, packet](boost::system::error_code ec, size_t length) {
                    if (!ec) {
                        spdlog::debug("Packet payload sent: {}", packet.payload);
                        writeHeader(packet);
                    } else {
                        spdlog::error("Send packet header error: {}. Close socket.", ec.message());
                        m_socket.close();
                    }
                });
}

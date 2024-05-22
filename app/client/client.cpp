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
    , m_socket{m_context} {}

[[maybe_unused]] bool Client::connect() {
    boost::system::error_code errorCode;
    m_socket.connect(m_endpoint, errorCode);

    if (errorCode) {
        spdlog::error("failed to connect, error: {}", errorCode.message());
        return false;
    }

    spdlog::info("Client connected");
    return true;
}

bool Client::sendFile(const std::string& fileName) {
    // 4 stages:
    // 1 - send file name
    // 2 - send data from file to socket
    // 3 - send hash of a file
    // 4 - send exit and finish

    FileHandler fileHandler;
    Packet packet;
    CryptoPacket cryptoPacket;
    boost::system::error_code ec;

    // stage 1 - send file name
//    {
        packet.header.type = Packet::Type::FileName;
        packet.header.length = fileName.size();
        memcpy(packet.payload, fileName.c_str(), fileName.size());

        cryptographer.encrypt(packet, cryptoPacket);

        if (!writeToSocketCrypto(m_socket, cryptoPacket, ec)) {
            return false;
        }
//    }

    // stage 2 - send data from file to socket
//    {
        fileHandler.open(fileName, "rb");
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
                spdlog::error("Didn't get ack packet for FileData packet");
            }
        } while (true);
//    }

    // stage 3 - send hash of a file
//    {
        packet.header.type = Packet::Type::Hash;
        const auto hash = fileHandler.getFileHash(fileName);
        memcpy(packet.payload, hash.c_str(), hash.size());
        packet.header.length = hash.size();
        if (!writeToSocket(m_socket, packet, ec)) {
            return false;
        }

        if (!readFromSocket(m_socket, packet, ec)) {
            spdlog::error("Didn't get ack packet for Hash packet");
        }
//    }

    // stage 4 - send exit and finish
//    {
        packet.header.type = Packet::Type::Exit;
        packet.header.length = 1;

        if (!writeToSocket(m_socket, packet, ec)) {
            return false;
        }

        if (!readFromSocket(m_socket, packet, ec)) {
            spdlog::error("Didn't get ack packet for Exit packet");
        }

        spdlog::debug("Exit packet was sent. Finish");
//    }

    return true;
}

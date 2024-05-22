/*! \file socket_file_connection.cpp
 * \brief SocketFileConnection class implementation.
 */


#include "socket_file_connection.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

SocketFileConnection::SocketFileConnection(tcp::socket socket) : m_socket(std::move(socket)) {
    _ack_packet.header.type = Packet::Type::Ack;
    _ack_packet.header.length = 0;
}

void SocketFileConnection::run() {
    _readHeader();
}

void SocketFileConnection::_readHeader() {
    //The server does not keep a copy of the shared-pointer, so when all async operations on the connection complete
    // (e.g. because the connection was dropped) the SocketFileConnection object will be freed.
    //BUT the object must not be destroyed when an async operation is in progress
    auto self(shared_from_this());
    // 20 packs
    // узкое место - сеть
    async_read(m_socket, boost::asio::buffer(&_cryptoPacket.header, sizeof(_cryptoPacket.header)),
               [this, self] (boost::system::error_code errorCode, std::size_t length) {
                   if (!errorCode) {
                       if (_cryptoPacket.header.length > 0) {
                           _readPayload();
                       } else {
                           _writeHeader(_ack_packet);
                       }
                   } else {
                       spdlog::error("error header: {}", errorCode.message());
                       m_socket.close();
                   }
               });
}

void SocketFileConnection::_readPayload() {
    auto self(shared_from_this());
    async_read(m_socket, boost::asio::buffer(&_cryptoPacket.payload, _cryptoPacket.header.length),
               [this, self] (boost::system::error_code errorCode, std::size_t length) {
                   if (!errorCode) {
                       _handlePacket();//gateway распределение мощности
                       _writeHeader(_ack_packet);
                   } else {
                       spdlog::error("error payload: {}", errorCode.message());
                       m_socket.close();
                   }
               });
}

void SocketFileConnection::_writeHeader(const Packet& packet) {
    auto self(shared_from_this());
    boost::asio::async_write(m_socket, boost::asio::buffer(&packet.header, sizeof(packet.header)),
                             [this, self, packet](boost::system::error_code errorCode, std::size_t /*length*/) {
                                 if (!errorCode) {
                                     _writePayload(packet);
                                 }
                             });
}

void SocketFileConnection::_writePayload(const Packet& packet) {
    auto self(shared_from_this());
    boost::asio::async_write(m_socket, boost::asio::buffer(&packet.payload, packet.header.length),
                             [this, self, packet](boost::system::error_code errorCode, std::size_t /*length*/) {
                                 if (!errorCode) {
                                     _readHeader();
                                 }
                             });
}

void SocketFileConnection::_handlePacket() {
    _packet = cryptographer.decrypt(_cryptoPacket);
    // add functions for every case
    switch (_packet.header.type) {
        case (Packet::Type::FileName): {
            // at this step we need to open file
            spdlog::info("Got a package with a file name");

            // cast char[] to string
            std::string fileName(_packet.payload, _packet.header.length);
            // ['/path/to/filename' -> 'filename' ] get only name without some paths
            if (fileName.find('/') != string::npos) {
                size_t lastSlashIndx = fileName.rfind('/');
                fileName = fileName.substr(lastSlashIndx + 1, fileName.size());
            }

            if (fileHandler.isFileExist(fileName)) {
                spdlog::debug("File with the name {} exists.", fileName);
                fileName = fileHandler.getUniqueName(fileName);
            }

            spdlog::debug("Open a file:  {}", fileName);
            fileHandler.open(fileName, "w");

            break;
        };
        case (Packet::Type::FileData): {
            // at this step we write data from socket to file
            fileHandler.write(_packet);
//            fileHandler.close();
//            cout << "DATA: " << _packet.payload << endl;
            break;
        };
        case (Packet::Type::Hash): {
            // at this step we generate a hash for current file and compare it with a hash that client sent to us

            // need to close file because it adds EOL
            fileHandler.close();

            spdlog::info("Generate a hash from file");
            // Hash of our file
            auto hash = fileHandler.getFileHash(fileHandler.getFilename());
            // Client's file hash
            auto clientFileHash = std::string(_packet.payload, _packet.header.length);
            if (hash != clientFileHash) {
                spdlog::error("Client file hash and our hash is different: {} vs {}", clientFileHash, hash);
            } else {
                spdlog::info("Files hashes are same.");
            }
            break;
        };
        case (Packet::Type::Exit): {
            // at this step we close the socket and exit
            spdlog::info("Client sent exit packet. Close socket");
            m_socket.close();
            break;
        };
        default: {
            spdlog::error("Unknown packet");
            break;
        }
    }
}

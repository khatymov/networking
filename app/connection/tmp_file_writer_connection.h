/*! \file file_writer_connection.h
 * \brief TmpFileWriterConnection class interface.
 *
 * The main purpose of this class is accept file data and write the data to a file
 *
 */


#pragma once

//#include "connection_interface.h"
#include "file_handler.h"
#include "common.h"


using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

/*! \class TmpFileWriterConnection
 * \brief Some briefing
 */
class TmpFileWriterConnection: public std::enable_shared_from_this<TmpFileWriterConnection> {
//    TmpFileWriterConnection(const TmpFileWriterConnection&) = delete;
//    TmpFileWriterConnection(TmpFileWriterConnection&&) = delete;
//    TmpFileWriterConnection operator=(const TmpFileWriterConnection&) = delete;
//    TmpFileWriterConnection operator=(TmpFileWriterConnection&&) = delete;
public:

    //! \brief default constructor.
    TmpFileWriterConnection(boost::asio::ip::tcp::socket socket) : m_socket(std::move(socket)) {
        ack_packet.header.type = Packet::Type::Ack;
        ack_packet.header.length = 0;
    }

    //! \brief default destructor.
    ~TmpFileWriterConnection() = default;

    //! \brief send/receive packets
//    void Process() override;

    //! \brief send/receive packets
//    void FileProcess();

    void run() {
        _ReadHeader();
    }

//    bool isPingable();
private:

    void _ReadHeader() {
        auto self(shared_from_this());
        async_read(m_socket, boost::asio::buffer(&_packet.header, sizeof(_packet.header)),
                   [this, self] (boost::system::error_code errorCode, std::size_t length) {
                       spdlog::debug("header len: {}", length);

                       if (!errorCode) {
                           spdlog::debug("Packet header: ");
                           if (_packet.header.length > 0) {
                               _ReadPayload();
                               spdlog::info("Here should be read body");
                           } else {
                               do_write(ack_packet);
                           }

                       } else {
                           spdlog::error("error header: {}", errorCode.message());
                           m_socket.close();
                       }
                   });
    }

    void _ReadPayload() {
        auto self(shared_from_this());
        async_read(m_socket, boost::asio::buffer(&_packet.payload, _packet.header.length),
                   [this, self] (boost::system::error_code errorCode, std::size_t length) {
                       spdlog::debug("payload len: {}", length);
                       if (!errorCode) {
                           spdlog::debug("Packet payload: {}", string(_packet.payload, _packet.header.length));
                           _ProcessPacket();
                           do_write(ack_packet);
                       } else {
                           spdlog::error("error payload: {}", errorCode.message());
                           m_socket.close();
                       }
                   });
    }

    void do_write(Packet& packet) {
        auto self(shared_from_this());
        boost::asio::async_write(m_socket, boost::asio::buffer(&packet.header, sizeof(packet.header)),
                                 [this, self, packet](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec) {
                                         do_write_body(ack_packet);
                                     }
                                 });
    }

    void do_write_body(Packet packet) {
        auto self(shared_from_this());
        boost::asio::async_write(m_socket, boost::asio::buffer(&packet.payload, packet.header.length),
                                 [this, self, packet](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec) {
                                         _ReadHeader();
                                     }
                                 });
    }

    void _ProcessPacket() {
        Packet ack_packet;
        ack_packet.header.length = 0;
        ack_packet.header.type = Packet::Type::Ack;
        boost::system::error_code ec;

        switch (_packet.header.type) {
            case (Packet::Type::FileName): {
                // at this step we need to open file
                spdlog::debug("Create a file");
                std::string fileName(_packet.payload, _packet.header.length);
                if (fileHandler.isFileExist(fileName)) {
                    spdlog::debug("File with the name {} exists.", fileName);
                    fileName = fileHandler.getUniqueName(fileName);
                    spdlog::debug("New file name {}.", fileName);
                }

                fileHandler.open(fileName, "w");

                break;
            };
            case (Packet::Type::FileData): {
                // at this step we write data from socket to file
                spdlog::debug("Read from socket and write to the file");

                fileHandler.write(_packet);

                break;
            };
            case (Packet::Type::Hash): {
                fileHandler.close();
                // at this step we generate a hash for current file and compare it with a hash that client sent to us
                spdlog::debug("Generate a hash from file");
                auto hash = fileHandler.getFileHash(fileHandler.getFilename());
                auto clientFileHash = std::string(_packet.payload, _packet.header.length);
                if (hash != clientFileHash) {
                    spdlog::error("Client file hash and our hash is different: {} vs {}", clientFileHash, hash);
                } else {
                    spdlog::debug("HASHES are same!!!!!!!");
                }
                break;
            };
            case (Packet::Type::Exit): {
                // at this step we close the socket and exit
                spdlog::debug("We are done");
                m_socket.close();
                break;
            };
            default: {
                spdlog::error("Unknown packet");
                break;
            }
        }
    }

    //! \brief Each connection has a unique socket to a remote
    boost::asio::ip::tcp::socket m_socket;
    FileHandler fileHandler;

    Packet _packet;
    Packet ack_packet;
    //! List of private variables.
};

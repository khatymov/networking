/*! \file file_writer_connection.cpp
 * \brief FileWriterConnection class implementation.
 */


#include "file_writer_connection.h"
#include "packet.h"
#include "socket_messenger/socket_messenger.h"

using namespace std;
using namespace boost::asio;

FileWriterConnection::FileWriterConnection(boost::asio::io_context& ioContext, boost::asio::ip::tcp::socket socket)
    : ConnectionInterface(ioContext, std::move(socket))
{
}

void FileWriterConnection::Process()
{
    Packet packet;
    boost::system::error_code ec;
    while (getSocket().is_open()){
        size_t size = read(getSocket(), boost::asio::buffer(&packet.payload, sizeof(packet.payload)), transfer_exactly(sizeof(packet.payload)), ec);

        if (!ec and size > 0) {
            spdlog::info("Client message: {}", packet.payload);
            continue;
        }

        getSocket().close();
        if ((!ec and size == 0) or ec == boost::asio::error::eof) {
            spdlog::info("Finish reading, close socket");
        } else if (ec) {
            spdlog::error("Error while reading client message: {}", ec.message());
        }

    }
}

FileWriterConnection::~FileWriterConnection()
{
    spdlog::info("~FileWriterConnection()");
}

bool FileWriterConnection::isPingable()
{
    spdlog::debug("Server, send ping");
    //send ping packet
    Packet packet;
    packet.header.type = Packet::Type::Ping;
    packet.header.length = 0;
    boost::system::error_code ec;

    if (!writeToSocket(getSocket(), packet, ec)) {
        return false;
    }

    spdlog::debug("Server, wait for pong packet");
    //wait for pong packet
    if (!readFromSocket(getSocket(), packet, ec)) {
        return false;
    }

    return packet.header.type == Packet::Type::Pong;
}
void FileWriterConnection::FileProcess()
{
    if (!isPingable()) {
        spdlog::error("Can't get a pong from a client");
    }

    spdlog::info("Client is available.");

    Packet packet;
    boost::system::error_code ec;

    while (getSocket().is_open()) {
        if (!readFromSocket(getSocket(), packet, ec)) {
            continue;
        }

        switch (packet.header.type)
        {
            case (Packet::Type::FileName): {
                // at this step we need to open file
                spdlog::debug("Create a file");
                std::string fileName(packet.payload);
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
                // read(socket, packet)
                // fileHandler.write(packet.data

                break;
            };
            case (Packet::Type::Hash): {
                // at this step we generate a hash for current file and compare it with a hash that client sent to us
                spdlog::debug("Generate a hash from file");

                break;
            };
            case (Packet::Type::Exit): {
                // at this step we close the socket and exit
                spdlog::debug("We are done");
                getSocket().close();
                break;
            };
            default: {
                spdlog::error("Unknown packet");
                break;
            }

        }
        // send: ping -> wait for:  pong <-
        // wait for: filename


        // wait for: file data
        // wait for: hash
        // wait for: exit
    }
}

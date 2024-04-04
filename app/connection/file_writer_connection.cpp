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
    // send: ping -> wait for:  pong <-
    // wait for: filename
    // wait for: file data
    // wait for: hash
    // wait for: exit

    if (!isPingable()) {
        spdlog::error("Can't get a pong from a client");
    }

    spdlog::info("Client is available.");

    Packet packet;
    boost::system::error_code ec;

    while (getSocket().is_open()){

    }
}

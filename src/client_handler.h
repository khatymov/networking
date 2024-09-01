//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_CLIENT_HANDLER_H
#define NETWORKING_CLIENT_HANDLER_H

#include "consoleParams.h"
#include "pch.h"
#include "connection.h"
#include "file_reader.h"

namespace network {

using namespace network;

class ClientHandler {
    ClientHandler(const ClientHandler&) = delete;
    ClientHandler(ClientHandler&&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
    ClientHandler& operator=(ClientHandler&&) = delete;
public:
    explicit ClientHandler(const ConsoleParams& params);
    void handle();

protected:
    //! \brief asio context handles the data transfer...
    boost::asio::io_context context_;
    //! \brief using to resolve hostname/ip-address into tangiable physical address
    boost::asio::ip::tcp::endpoint endpoint_;
    //! \brief socket allows us to connect to the server
    boost::asio::ip::tcp::socket socket_;

    std::vector<std::shared_ptr<ThreadSafeQueue<CryptoPacket>>> tsQueues_;
    std::unique_ptr<Connection<CryptoPacket>> connection_;
    std::unique_ptr<FileReader<CryptoPacket>> fileReader_;
};

}  // network

#endif  // NETWORKING_CLIENT_HANDLER_H

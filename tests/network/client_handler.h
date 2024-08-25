//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_CLIENT_HANDLER_H
#define NETWORKING_CLIENT_HANDLER_H

#include "pch.h"

namespace network {

using namespace boost::asio::ip;

class ClientHandler {
    ClientHandler(const ClientHandler&) = delete;
    ClientHandler(ClientHandler&&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
    ClientHandler& operator=(ClientHandler&&) = delete;
public:
    explicit ClientHandler(tcp::socket socket);
    void handle();
};

}  // network

#endif  // NETWORKING_CLIENT_HANDLER_H

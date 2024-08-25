//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_SERVER_H
#define NETWORKING_SERVER_H

#include "pch.h"

#include "consoleParams.h"

namespace network {

using namespace boost::asio;

class Server {
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
public:
    // create acceptor
    explicit Server(const ConsoleParams& params);
    // start to accept new connection(clients) and send socket to client handler
    void handleConnections();
protected:
    io_context ioContext_;
    ip::tcp::acceptor acceptor_;
};

} // namespace network


#endif  // NETWORKING_SERVER_H

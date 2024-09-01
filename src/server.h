//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_SERVER_H
#define NETWORKING_SERVER_H

#include "pch.h"


#include "session.h"
#include "my_packet.h"
#include "consoleParams.h"

namespace network {

using namespace boost::asio;
using namespace boost::asio::ip;

template <typename T>
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

//create acceptor
template <typename T>
Server<T>::Server(const ConsoleParams& params)
    : acceptor_(ioContext_, tcp::endpoint(make_address(params.ip.data()), params.port)) {
    spdlog::info("Server started with ip and port: [{}:{}]", params.ip.data(), params.port);
}

template <typename T>
void Server<T>::handleConnections() {
    while (true) {
        tcp::socket socket(ioContext_);
        spdlog::info("Waiting for a connection...");

        // Synchronously accept a new connection
        acceptor_.accept(socket);
        spdlog::info("New connection accepted");

        // Create a new thread for each session
        std::thread client_thread([&socket]() {
            Session<T> session(std::move(socket));
            session.handle();
        });

        // Detach the thread so it can run independently
        client_thread.detach();
    }
}

} // namespace network


#endif  // NETWORKING_SERVER_H

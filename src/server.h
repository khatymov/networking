//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_SERVER_H
#define NETWORKING_SERVER_H

#include "pch.h"

#include "my_packet.h"
#include "connection.h"
#include "consoleParams.h"
#include "defs.h"
#include "my_packet.h"
#include "thread_safe_queue.h"

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
    explicit Server(const ConsoleParams& params, boost::asio::io_context& ioContext);
    // start to accept new connection(clients) and send socket to client handler
    void handleConnections();

protected:
    io_context ioContext_;
    ip::tcp::acceptor acceptor_;
};

//create acceptor
template <typename T>
Server<T>::Server(const ConsoleParams& params, boost::asio::io_context& ioContext)
    : acceptor_(ioContext, tcp::endpoint(make_address(params.ip.data()), params.port)) {
    spdlog::info("Server started with ip and port: [{}:{}]", params.ip.data(), params.port);
}

template <typename T>
void Server<T>::handleConnections() {
    spdlog::info("Waiting for a connection...");
    acceptor_.async_accept([this](boost::system::error_code errorCode, tcp::socket socket){
        if (!errorCode) {
            spdlog::info("New connection accepted");
            std::vector<std::shared_ptr<ThreadSafeQueue<T>>> tsQueues = {std::make_shared<ThreadSafeQueue<T>>(true) // <- Connection
                ,std::make_shared<ThreadSafeQueue<T>>(false) // <- Decryptor
                ,std::make_shared<ThreadSafeQueue<T>>(false) // <- FileWriter
            };
            std::make_shared<Connection<T>>(Mode::Server, std::move(socket), std::move(tsQueues), tsQueues[0], tsQueues[1])->run();
        } else {
            spdlog::error("Error has occurred during acceptance: {}", errorCode.message());
        }

        handleConnections();
    });


}

} // namespace network


#endif  // NETWORKING_SERVER_H

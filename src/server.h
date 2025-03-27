//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_SERVER_H
#define NETWORKING_SERVER_H

#include "consoleParams.h"
#include "defs.h"
#include "my_packet.h"
#include "pch.h"
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
        explicit Server(const ConsoleParams& params);
        // start to accept new connection(clients) and send socket to client handler
        void handleConnections();

    protected:
        io_context ioContext_;
        ip::tcp::acceptor acceptor_;
    };

    // create acceptor
    template <typename T>
    Server<T>::Server(const ConsoleParams& params)
        : acceptor_(ioContext_, tcp::endpoint(make_address(params.ip.data()), params.port)) {
        spdlog::info("Server started with ip and port: [{}:{}]", params.ip.data(), params.port);
    }

    template <typename T>
    void Server<T>::handleConnections() {
        spdlog::info("Waiting for a connection...");
        acceptor_.async_accept([this](boost::system::error_code errorCode, tcp::socket socket) {
            if (!errorCode) {
                spdlog::info("New connection accepted");
                std::vector<QueuePtr<T>> tsQueues = {
                    std::make_shared<ThreadSafeQueue<T>>(true)  // <- Connection (Primary)
                    ,
                    std::make_shared<ThreadSafeQueue<T>>(false)  // <- Decryptor
                    ,
                    std::make_shared<ThreadSafeQueue<T>>(false)  // <- FileWriter
                };

                NamedQueue<T> namedQueues = {{ConnectionKey, {tsQueues[0], tsQueues[1]}},
                                             {DecryptorKey, {tsQueues[1], tsQueues[2]}},
                                             {FileWriterKey, {tsQueues[2], tsQueues[0]}}};

                std::make_shared<Connection<T>>(std::move(socket), std::move(namedQueues))->run();
            } else {
                spdlog::error("Error has occurred during acceptance: {}", errorCode.message());
            }

            handleConnections();
        });

        ioContext_.run();
    }

}  // namespace network

#endif  // NETWORKING_SERVER_H

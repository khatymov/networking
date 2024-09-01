//
// Created by Renat_Khatymov on 8/22/2024.
//

#include "client_handler.h"
#include "connection.h"

using namespace boost::asio;
using namespace boost::asio::ip;

namespace network {


ClientHandler::ClientHandler(const ConsoleParams& params)
    :endpoint_{address::from_string(params.ip.data()),
               static_cast<port_type>(params.port)}
    ,socket_{context_} {

    // queues should the same number as number of components
    // one queue is primary ~ all memory for packets allocates in that queue
    tsQueues_ = {std::make_shared<ThreadSafeQueue<CryptoPacket>>(true),
                 std::make_shared<ThreadSafeQueue<CryptoPacket>>(false)};

    // reader is entry point. All works starts from reading from file and then send it to connection component
    fileReader_ = std::make_unique<FileReader<CryptoPacket>>(params.targetFile.data(), tsQueues_[0], tsQueues_[1]);

    boost::system::error_code errorCode;
    socket_.connect(endpoint_, errorCode);
    if (errorCode) {
        throw std::runtime_error("Socket could not connect to the server");
    }
    connection_ = std::make_unique<Connection<CryptoPacket>>(Mode::Client, std::move(socket_), tsQueues_[1], tsQueues_[0]);
}

void ClientHandler::handle() {
    std::vector<std::thread> threads;

    threads.emplace_back([fileReader = std::move(fileReader_)](){
        // send file name to connection
        fileReader->waitNextData();
        fileReader->setFileName();
        fileReader->notifyComplete();
        // send data
        while (not fileReader->isDone()) {
            fileReader->waitNextData();
            fileReader->processData();
            fileReader->notifyComplete();
        }
        // send file's hash to connection
        fileReader->waitNextData();
        fileReader->setFileName();
        fileReader->notifyComplete();
    });

    threads.emplace_back([connection = std::move(connection_)](){
        while (not connection->isDone()) {
            connection->waitNextData();
            connection->processData();
            connection->notifyComplete();
        }
    });

    for (auto& th: threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

}  // network
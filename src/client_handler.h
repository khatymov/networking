//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_CLIENT_HANDLER_H
#define NETWORKING_CLIENT_HANDLER_H

#include "consoleParams.h"
#include "pch.h"
#include "connection.h"
#include "file_reader.h"
#include "encryptor.h"
#include "timer.h"

namespace network {

using namespace network;

template <typename T>
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
    //! \brief calculate time of handling
    Timer timer_;

    std::vector<std::shared_ptr<ThreadSafeQueue<T>>> tsQueues_;
    std::unique_ptr<FileReader<T>> fileReader_;
    std::unique_ptr<Encryptor<T>> encryptor_;
    std::shared_ptr<Connection<T>> connection_;
};

using namespace boost::asio;
using namespace boost::asio::ip;

template <typename T>
ClientHandler<T>::ClientHandler(const ConsoleParams& params)
    :endpoint_(tcp::endpoint(
          boost::asio::ip::make_address(params.ip.data()),
          static_cast<port_type>(params.port)))
    ,socket_{context_} {

    // queues should the same number as number of components
    // one queue is primary ~ all memory for packets allocates in that queue
    tsQueues_ = {std::make_shared<ThreadSafeQueue<T>>(true) // <- FileReader
                ,std::make_shared<ThreadSafeQueue<T>>(false) // <- Encryptor
                ,std::make_shared<ThreadSafeQueue<T>>(false) // <- Connection
                 };

    NamedQueue<T> namedQueues = {{FileReaderKey, {tsQueues_[0], tsQueues_[1]}},
                                 {EncryptorKey,  {tsQueues_[1], tsQueues_[2]}},
                                 {ConnectionKey, {tsQueues_[2], tsQueues_[0]}}};

    // reader is entry point. All works starts from reading from file and then send it to connection component
    fileReader_ = std::make_unique<FileReader<T>>(params.targetFile.data(),
                                                  namedQueues[FileReaderKey].first,
                                                  namedQueues[FileReaderKey].second);

    encryptor_ = std::make_unique<Encryptor<T>>(namedQueues[EncryptorKey].first, namedQueues[EncryptorKey].second);

    boost::system::error_code errorCode;
    socket_.connect(endpoint_, errorCode);
    if (errorCode) {
        throw std::runtime_error("Socket could not connect to the server");
    }
    connection_ = std::make_shared<Connection<T>>(std::move(socket_),
                                                  namedQueues[ConnectionKey].first,
                                                  namedQueues[ConnectionKey].second);
}

template <typename T>
void ClientHandler<T>::handle() {
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
        fileReader->setHash();
        fileReader->notifyComplete();

        // send file's hash to connection
        fileReader->waitNextData();
        fileReader->setExitPack();
        fileReader->notifyComplete();
    });

    threads.emplace_back([encryptor = std::move(encryptor_)](){
        while (not encryptor->isDone()) {
            encryptor->waitNextData();
            encryptor->processData();
            encryptor->notifyComplete();
        }
    });

    threads.emplace_back([connection = connection_](){
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

#endif  // NETWORKING_CLIENT_HANDLER_H

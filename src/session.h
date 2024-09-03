//
// Created by Renat_Khatymov on 9/2/2024.
//

#ifndef NETWORK_SESSION_H
#define NETWORK_SESSION_H

#include "pch.h"

#include "connection.h"
#include "file_writer.h"
#include "decryptor.h"

namespace network {

template <typename T>
class Session {
    Session(const Session&) = delete;
    Session(Session&&) = delete;
    Session& operator = (const Session&) = delete;
    Session& operator = (Session&&) = delete;
public:

    Session(boost::asio::ip::tcp::socket socket);

    void handle();

protected:
    std::vector<std::shared_ptr<ThreadSafeQueue<T>>> tsQueues_;

    std::unique_ptr<Connection<T>> connection_;
    std::unique_ptr<Decryptor<T>> decryptor_;
    std::unique_ptr<FileWriter<T>> fileWriter_;
};

template <typename T>
Session<T>::Session(boost::asio::ip::tcp::socket socket) {
    // queues should the same number as number of components
    // one queue is primary ~ all memory for packets allocates in that queue
    tsQueues_ = {std::make_shared<ThreadSafeQueue<T>>(true) // <- Connection
                ,std::make_shared<ThreadSafeQueue<T>>(false) // <- Decryptor
                ,std::make_shared<ThreadSafeQueue<T>>(false) // <- FileWriter
    };

    // Connection is entry point.
    // All works starts from reading from socket and then send it to the next component
    connection_ = std::make_unique<Connection<T>>(Mode::Server, std::move(socket), tsQueues_[0], tsQueues_[1]);
    decryptor_ = std::make_unique<Decryptor<T>>(tsQueues_[1], tsQueues_[2]);
    fileWriter_ = std::make_unique<FileWriter<T>>(tsQueues_[2], tsQueues_[0]);
}

template <typename T>
void Session<T>::handle() {
    std::vector<std::thread> threads;

    threads.emplace_back([connection = std::move(connection_)](){
        while (not connection->isDone()) {
            connection->waitNextData();
            connection->processData();
            connection->notifyComplete();
        }
    });

    threads.emplace_back([decryptor = std::move(decryptor_)](){
        while (not decryptor->isDone()) {
            decryptor->waitNextData();
            decryptor->processData();
            decryptor->notifyComplete();
        }
    });

    threads.emplace_back([fileWriter = std::move(fileWriter_)](){
        while (not fileWriter->isDone()) {
            fileWriter->waitNextData();
            fileWriter->processData();
            fileWriter->notifyComplete();
        }
    });

    for (auto& th: threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

}  // network

#endif  // NETWORK_SESSION_H

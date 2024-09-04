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
class Session: public std::enable_shared_from_this<Session<T>> {
    Session(const Session&) = delete;
    Session(Session&&) = delete;
    Session& operator = (const Session&) = delete;
    Session& operator = (Session&&) = delete;
public:

    Session(std::vector<std::shared_ptr<ThreadSafeQueue<T>>>& tsQueues_);

    ~Session();
//    Session(boost::asio::ip::tcp::socket socket);

    void handle();

protected:


    std::shared_ptr<Connection<T>> connection_;
    std::unique_ptr<Decryptor<T>> decryptor_;
    std::unique_ptr<FileWriter<T>> fileWriter_;
    std::vector<std::thread> threads;
};
template <typename T>
Session<T>::~Session() {
    for (auto& th: threads) {
        th.join();
    }
}

template <typename T>
Session<T>::Session(std::vector<std::shared_ptr<ThreadSafeQueue<T>>>& tsQueues_) {
//    Session<T>::Session(boost::asio::ip::tcp::socket socket) {
    // queues should the same number as number of components
    // one queue is primary ~ all memory for packets allocates in that queue
//    tsQueues_ = {std::make_shared<ThreadSafeQueue<T>>(true) // <- Connection
//                ,std::make_shared<ThreadSafeQueue<T>>(false) // <- Decryptor
//                ,std::make_shared<ThreadSafeQueue<T>>(false) // <- FileWriter
//    };

    // Connection is entry point.
    // All works starts from reading from socket and then send it to the next component

    decryptor_ = std::make_unique<Decryptor<T>>(tsQueues_[1], tsQueues_[2]);
    fileWriter_ = std::make_unique<FileWriter<T>>(tsQueues_[2], tsQueues_[0]);
}

template <typename T>
void Session<T>::handle() {

//    threads.emplace_back([connection = connection_](){
//
//    });

    threads.emplace_back([decryptor = std::move(decryptor_)](){
        while (not decryptor->isDone()) {
            decryptor->waitNextData();
            decryptor->processData();
            decryptor->notifyComplete();
        }
        spdlog::debug("decryptor->isDone()");
    });

    threads.emplace_back([fileWriter = std::move(fileWriter_)](){
        while (not fileWriter->isDone()) {
            fileWriter->waitNextData();
            fileWriter->processData();
            fileWriter->notifyComplete();
        }
        spdlog::debug("fileWriter->isDone()");
    });

//    spdlog::debug("Connection begin");
//    std::make_shared<Connection<T>>(Mode::Server, std::move(socket), tsQueues_[0], tsQueues_[1])->run();
//    spdlog::debug("Connection end");


}

}  // network

#endif  // NETWORK_SESSION_H

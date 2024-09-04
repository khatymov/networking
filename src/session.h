//
// Created by Renat_Khatymov on 9/2/2024.
//

#ifndef NETWORK_SESSION_H
#define NETWORK_SESSION_H

#include "pch.h"

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

    void handle();
protected:
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
    decryptor_ = std::make_unique<Decryptor<T>>(tsQueues_[1], tsQueues_[2]);
    fileWriter_ = std::make_unique<FileWriter<T>>(tsQueues_[2], tsQueues_[0]);
}

template <typename T>
void Session<T>::handle() {
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
}

}  // network

#endif  // NETWORK_SESSION_H

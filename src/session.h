//
// Created by Renat_Khatymov on 9/2/2024.
//

#ifndef NETWORK_SESSION_H
#define NETWORK_SESSION_H

#include "decryptor.h"
#include "file_writer.h"
#include "pch.h"

namespace network {

    template <typename T>
    class Session : public std::enable_shared_from_this<Session<T>> {
        Session(const Session&) = delete;
        Session(Session&&) = delete;
        Session& operator=(const Session&) = delete;
        Session& operator=(Session&&) = delete;

    public:
        explicit Session(NamedQueue<T> namedQueues);

        ~Session();

        void handle();

    protected:
        std::unique_ptr<Decryptor<T>> decryptor_;
        std::unique_ptr<FileWriter<T>> fileWriter_;
        std::vector<std::thread> threads;
    };

    template <typename T>
    Session<T>::~Session() {
        for (auto& th : threads) {
            th.join();
        }
    }

    template <typename T>
    Session<T>::Session(NamedQueue<T> namedQueues) {
        decryptor_ = std::make_unique<Decryptor<T>>(namedQueues[DecryptorKey].first,
                                                    namedQueues[DecryptorKey].second);
        fileWriter_ = std::make_unique<FileWriter<T>>(namedQueues[FileWriterKey].first,
                                                      namedQueues[FileWriterKey].second);
    }

    template <typename T>
    void Session<T>::handle() {
        threads.emplace_back([decryptor = std::move(decryptor_)]() {
            while (not decryptor->isDone()) {
                decryptor->waitNextData();
                decryptor->processData();
                decryptor->notifyComplete();
            }
        });

        threads.emplace_back([fileWriter = std::move(fileWriter_)]() {
            while (not fileWriter->isDone()) {
                fileWriter->waitNextData();
                fileWriter->processData();
                fileWriter->notifyComplete();
            }
        });
    }

}  // network

#endif  // NETWORK_SESSION_H

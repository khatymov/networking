//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_DATA_PROCESSOR_INTERFACE_H
#define NETWORKING_DATA_PROCESSOR_INTERFACE_H

#include "pch.h"

#include "thread_safe_queue.h"

namespace network {
template <typename Derived, typename T>
class DataProcessor {
public:
    // get current and next queue to get data from current queue, handle this data and give it to the next queue
    explicit DataProcessor(std::shared_ptr<ThreadSafeQueue<T>>& currentQueue,
                            std::shared_ptr<ThreadSafeQueue<T>>& nextQueue)
        :currentQueue_(currentQueue),
         nextQueue_(nextQueue),
         tsStart_(std::chrono::system_clock::now()){}

    // wait data from current queue
    void waitNextData();
    // do something with data from current queue
    void processData();
    // give this data to the next queue
    void notifyComplete();
    //this function will be used to verify that we are done with data processing
    bool isDone() const;
protected:
    std::shared_ptr<ThreadSafeQueue<T>> currentQueue_;
    std::shared_ptr<ThreadSafeQueue<T>> nextQueue_;
    std::unique_ptr<T> data_;
    // this flag is a sign that component is finished its work
    // flag value should be changes in processData() in child class
    bool isProcessDone_ = false;

    std::chrono::time_point<std::chrono::system_clock> tsStart_;
};
//TODO delete
template <typename Derived, typename T>
void DataProcessor<Derived, T>::waitNextData() {
    // while data
    while ((data_ = currentQueue_->get()) == nullptr) {
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tsStart_);
        if (duration.count() > 100) {
//            std::cerr << "Wait for too long" << std::endl;
            throw std::runtime_error("Wait for too long");
        }
        std::this_thread::yield();
        continue;
    }

    tsStart_ = std::chrono::system_clock::now();
    if (data_ == nullptr) {
        throw std::logic_error("Data ptr can't be null");
    }
}

template <typename Derived, typename T>
void DataProcessor<Derived, T>::processData() {
    static_cast<Derived*>(this)->processDataImpl();
}

template <typename Derived, typename T>
void DataProcessor<Derived, T>::notifyComplete() {
    nextQueue_->set(std::move(data_));
}

template <typename Derived, typename T>
bool DataProcessor<Derived, T>::isDone() const {
    return isProcessDone_;
}

} // namespace network

#endif  // NETWORKING_DATA_PROCESSOR_INTERFACE_H

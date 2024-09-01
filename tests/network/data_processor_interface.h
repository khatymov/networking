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
         nextQueue_(nextQueue) {}

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
};

template <typename Derived, typename T>
void DataProcessor<Derived, T>::waitNextData() {
    // while data
    while ((data_ = currentQueue_->get()) == nullptr) {
        std::this_thread::yield();
        continue;
    }

    assert(data_ != nullptr);
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

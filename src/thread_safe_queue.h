//
// Created by Renat_Khatymov on 8/21/2024.
//

#ifndef NETWORKING_THREAD_SAFE_QUEUE_H
#define NETWORKING_THREAD_SAFE_QUEUE_H

#ifndef QUEUE_DATA_NUM
#define QUEUE_DATA_NUM 3
#endif

#include "pch.h"

namespace network {

// entity of a class should be a smart pointer
// please, do not create it on a stack

template <typename T>
class ThreadSafeQueue {
public:
    // initially only one ThreadSafeQueue should create data that afterwords will be used in others
    // ThreadSafeQueue. Only that object of ThreadSafeQueue should have isPrimaryQueue=true
    /**
    * @brief Constructor
    */
    explicit ThreadSafeQueue(bool isPrimaryQueue);
    //! \brief get data from queue
    std::unique_ptr<T> get();
    //! \brief set data to queue
    void set(std::unique_ptr<T>&& data);

protected:
    // queue with data allocated on a heap
    std::queue<std::unique_ptr<T>> queue_;
    // mutex to protect data race among different threads
    std::mutex mutex_;
};

template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(bool isPrimaryQueue) {
    // we should create data once, in primary queue
    if (isPrimaryQueue) {
        for (int i = 0; i < QUEUE_DATA_NUM; ++i) {
            // fill queue with empty data
            queue_.push(std::make_unique<T>());
        }
    }
}

template <typename T>
std::unique_ptr<T> ThreadSafeQueue<T>::get() {
    std::unique_ptr<T> res;
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        if (not queue_.empty()) {
            res = std::move(queue_.front());
            queue_.pop();
        }
    }

    return std::move(res);
}

template <typename T>
void ThreadSafeQueue<T>::set(std::unique_ptr<T>&& data) {
    if (data == nullptr) {
#ifdef DEBUG
        std::cerr << "set data can't be null";
#endif
        return;
    }

//    if (queue_.size() + 1 > QUEUE_DATA_NUM) {
//        throw std::runtime_error("Queue size can't be greater QUEUE_DATA_NUM.");
//    }

    std::lock_guard<std::mutex> lockGuard(mutex_);
    queue_.push(std::move(data));
}

} // namespace network

#endif  // NETWORKING_THREAD_SAFE_QUEUE_H

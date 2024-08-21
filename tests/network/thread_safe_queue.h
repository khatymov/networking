//
// Created by Renat_Khatymov on 8/21/2024.
//

#ifndef NETWORKING_THREAD_SAFE_QUEUE_H
#define NETWORKING_THREAD_SAFE_QUEUE_H

#include "my_packet.h"

template <typename T= MyPacket<char>>
class ThreadSafeQueue {
public:
    // initially only one ThreadSafeQueue should create data that afterwords will be used in others
    // ThreadSafeQueue. Only that object of ThreadSafeQueue should have isMainQueue=true
    explicit ThreadSafeQueue(bool isMainQueue);
    //! \brief get data from queue
    std::unique_ptr<T> get();
    //! \brief set data to queue
    void set(std::unique_ptr<T> data);
private:
};


template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(bool isMainQueue) {

}

template <typename T>
std::unique_ptr<T> ThreadSafeQueue<T>::get() {
    return nullptr;
}

template <typename T>
void ThreadSafeQueue<T>::set(std::unique_ptr<T> data) {

}


#endif  // NETWORKING_THREAD_SAFE_QUEUE_H

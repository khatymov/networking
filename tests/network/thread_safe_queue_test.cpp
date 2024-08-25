//
// Created by Renat_Khatymov on 8/25/2024.
//

#include "thread_safe_queue_test.h"

#include <iostream>

#include "my_packet.h"
#include "thread_safe_queue.h"

#include "gtest/gtest.h"

using namespace std;
using namespace testing;
using namespace network;

// Start with the smallest testable pieces
// test edge cases

// Test thread safe queue
// create a mock class to add some methods to ThreadSafeQueue
template <typename T = int>
class ThreadSafeQueueMock: public ThreadSafeQueue<T>{
public:
    // initially only one ThreadSafeQueue should create data that afterwords will be used in others
    // ThreadSafeQueue. Only that object of ThreadSafeQueue should have isPrimaryQueue=true
    /**
    * @brief Constructor
     */
    explicit ThreadSafeQueueMock(bool isPrimaryQueue) : ThreadSafeQueue<T>(isPrimaryQueue) {}

    auto getQueueSize() const {
        return this->queue_.size();
    }
};

// Test in one thread
// put some value to queue e.g. ptr of int 1 2 3
// get these ptrs
// verify that queue is empty
TEST(test_thread_safe_queue, test_set_qet) {
    std::shared_ptr<ThreadSafeQueueMock<int>> tsQueue = std::make_shared<ThreadSafeQueueMock<int>>(true);
    std::queue<std::unique_ptr<int>> valsQueue;
    for (int i = 0; i < QUEUE_DATA_NUM; i++) {
        auto data_ptr = tsQueue->get();
        *data_ptr = i;
        valsQueue.push(std::move(data_ptr));
    }

    EXPECT_EQ(tsQueue->getQueueSize(), 0);
    EXPECT_EQ(tsQueue->get(), nullptr);

    for (int i = 0; i < QUEUE_DATA_NUM; i++) {
        auto data_ptr = std::move(valsQueue.front());
        valsQueue.pop();
        tsQueue->set(std::move(data_ptr));
    }

    EXPECT_EQ(tsQueue->getQueueSize(), QUEUE_DATA_NUM);
}

// add text where we put more than capacity and catch the exception
TEST(test_thread_safe_queue, test_overflow_exception) {
    std::shared_ptr<ThreadSafeQueueMock<int>> tsQueue = std::make_shared<ThreadSafeQueueMock<int>>(true);
    EXPECT_THROW(tsQueue->set(std::move(make_unique<int>(42))), std::runtime_error);
}

// add set(nullptr) - handle this case

// Test in two threads
// in the first thread we have primary queue
// wait/get data ptr. Do not do anything, just set data ptr back
// the last value is 8. When a loop will reach this value - exit
// in the second thread we have a queue that cosumes value from primary queue
// wait/get data ptr and set values from 0-8
// the last value is 8. When a loop will reach this value - exit
// verify that values are same
// verify that queues are empty
TEST(test_thread_safe_queue, test_two_thread) {
    size_t numComponents = 2;
    std::vector<std::thread> threads;
    threads.reserve(numComponents);
    std::vector<std::shared_ptr<ThreadSafeQueueMock<int>>> tsQueues = {std::make_shared<ThreadSafeQueueMock<int>>(true),
                                                                       std::make_shared<ThreadSafeQueueMock<int>>(false)};
    // final value that after sending data from one queue to another this value will be a sign of finish
    const int finalVal = 8;

    threads.emplace_back([curQueue = tsQueues[0], nextQueue = tsQueues[1], finalVal] () {
        unique_ptr<int> data;
        bool isDone = false;
        while (data == nullptr) {
            data = std::move(curQueue->get());

            if (data != nullptr and *data == finalVal) {
                isDone = true;
            }

            nextQueue->set(std::move(data));

            if (isDone) {
                break;
            }
        }
    });

    threads.emplace_back([curQueue = tsQueues[1], nextQueue = tsQueues[0], finalVal] () {
        unique_ptr<int> data;
        bool isDone = false;
        int dataVal = 0;
        while (data == nullptr) {
            data = std::move(curQueue->get());

            if (data != nullptr) {
                *data = ++dataVal;
                if (*data == finalVal) {
                    isDone = true;
                }
            }

            nextQueue->set(std::move(data));

            if (isDone) {
                break;
            }
        }
    });

    for (auto& curThread: threads) {
        curThread.join();
    }

    for (int i = finalVal - QUEUE_DATA_NUM + 1; i <= finalVal; ++i) {
        auto dataPtr = tsQueues[1]->get();
        EXPECT_EQ(i, *dataPtr);
    }

    EXPECT_EQ(tsQueues[0]->getQueueSize(), 0);
    EXPECT_EQ(tsQueues[1]->getQueueSize(), 0);
}

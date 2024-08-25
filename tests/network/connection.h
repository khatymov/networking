//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_CONNECTION_H
#define NETWORKING_CONNECTION_H

#include "pch.h"

#include "data_processor_interface.h"

namespace network {
// main goal for this class is to get/send packet from/to client
template <typename DataType = MyPacket<>>
class Connection: public DataProcessor<Connection<>> {
    Connection(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator = (const Connection&) = delete;
    Connection& operator = (Connection&&) = delete;
public:
    // to start communicate we need socket
    explicit Connection(boost::asio::ip::tcp::socket socket,
                        std::shared_ptr<ThreadSafeQueue<DataType>>& currentQueue,
                        std::shared_ptr<ThreadSafeQueue<DataType>>& nextQueue);
    // get data from socket
    void processData();

protected:
    std::unique_ptr<DataType> data_;
};
} // namespace network

#endif  // NETWORKING_CONNECTION_H

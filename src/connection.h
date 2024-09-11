//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_CONNECTION_H
#define NETWORKING_CONNECTION_H

#include "pch.h"

#include "defs.h"
#include "data_processor_interface.h"
#include "my_packet.h"
#include "session.h"

namespace network {

// main goal for this class is to get/send packet from/to client
// DataProcessor is base class with implemented 2 functions:
    // waitNextData - wait available data for current queue (some other component should give it to us)
    // notifyComplete - give data that we got in processDataImpl to the next component

// Connection is entry point for server.
// All works starts from reading from socket and then send it to the next component

template <typename DataType>
class Connection: public std::enable_shared_from_this<Connection<DataType>>, public DataProcessor<Connection<DataType>, DataType> {
    Connection(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator = (const Connection&) = delete;
    Connection& operator = (Connection&&) = delete;
public:
    // to start communicate we need socket
    explicit Connection(const Mode mode, boost::asio::ip::tcp::socket socket, const std::pair<uint, uint>& connectionIndexes,
                        std::vector<std::shared_ptr<ThreadSafeQueue<DataType>>> tsQueues);

    void processDataImpl();

    // read from/to socket
    // TODO: make them async
    [[maybe_unused]] static bool read(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& data, boost::system::error_code& ec);
    [[maybe_unused]] static bool write(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& data, boost::system::error_code& ec);

    ~Connection() = default;
    void run();

    void readHead();
    void readPayload();

    void writeHead();
//    void writePayload();

    std::function<void(boost::asio::ip::tcp::socket& socket,
                       std::unique_ptr<DataType>& data)> handler;

    // to be created once and reuse it
    // TODO move somewhere else
    static constexpr DataType ackPacket{{Header::Type::Ack}};
    static constexpr DataType nackPacket{{Header::Type::Nack}};
    std::unique_ptr<DataType> ackPackPtr;
    std::unique_ptr<DataType> nackPackPtr;

protected:
    boost::asio::ip::tcp::socket socket_;

    std::shared_ptr<Session<DataType>> session;
};

template <typename DataType>
Connection<DataType>::Connection(const Mode mode, boost::asio::ip::tcp::socket socket,
                                 const std::pair<uint, uint>& connectionIndexes,
                                 std::vector<std::shared_ptr<ThreadSafeQueue<DataType>>> tsQueues)
    :DataProcessor<Connection<DataType>, DataType>(tsQueues[connectionIndexes.first], tsQueues[connectionIndexes.second])
      ,socket_(std::move(socket))
      ,ackPackPtr(std::make_unique<DataType>(ackPacket))
      ,nackPackPtr(std::make_unique<DataType>(nackPacket))
      ,session(std::make_shared<Session<DataType>>(tsQueues)){
    session->handle();
}

template <typename DataType>
void Connection<DataType>::run() {
    readHead();
}

template <typename DataType>
void Connection<DataType>::readHead() {
    //????
    this->waitNextData();
    auto self(this->shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(&this->data_->header, sizeof(this->data_->header)),
                            //    boost::asio::async_read(socket_, boost::asio::buffer(&packet.header, sizeof(packet.header)),
                            [this, self] (boost::system::error_code errorCode, std::size_t length) {
//                                spdlog::debug("Got pack with len: {} or {}", length, this->data_->header.length);
                                if (!errorCode) {
                                    if (this->data_->header.type == Header::Type::Exit) {
                                        this->isProcessDone_ = true;
                                    }

                                    if (this->data_->header.length > 0) {
                                        readPayload();
                                    } else {
                                        //!!! Be cautious
                                        //                                           Connection<DataType>::write(socket_, ackPackPtr, errorCode);
                                    }
                                } else {
                                    spdlog::error("error header: {}", errorCode.message());
                                    socket_.close();
                                }
                            });
}

template <typename DataType>
void Connection<DataType>::readPayload() {
    auto self(this->shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(&this->data_->data, this->data_->header.length),
                            [this, self] (boost::system::error_code errorCode, std::size_t length) {
                                if (!errorCode) {
                                    if (this->data_->header.length > 0) {
                                        writeHead();
                                        //                                        Connection<DataType>::write(socket_, ackPackPtr, errorCode);
                                    } else {
                                        //!!! Be cautious
                                    }
                                } else {
                                    //                                    Connection::write(socket_, nackPackPtr, errorCode);
                                    spdlog::error("error header: {}", errorCode.message());
                                    socket_.close();
                                }
                            });
}

template <typename DataType>
void Connection<DataType>::writeHead() {
    auto self(this->shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(&ackPackPtr->header, sizeof(ackPackPtr->header)),
                             [this, self](boost::system::error_code errorCode, std::size_t /*length*/) {
                                 if (!errorCode) {
                                     this->notifyComplete();
                                     //!!! Be cautious
                                     if (not this->isDone()) {
                                         readHead();
                                     }
                                 } else {
                                     spdlog::error("error send header: {}", errorCode.message());
                                 }
                             });
}

//template <typename DataType>
//void Connection<DataType>::writePayload() {
//
//}


template <typename DataType>
void Connection<DataType>::processDataImpl() {
    try {
        handler(socket_, this->data_);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    } // add some others
}

// TODO read write should work with any type: complex structures, stl containers or boost buffer
template <typename DataType>
bool Connection<DataType>::write(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& packet, boost::system::error_code& ec) {
    // TODO: fix
//    auto* raw_header_ptr = reinterpret_cast<char*>(&packet->header);
    //send header
    auto sentHeaderSize = boost::asio::write(socket,
                                             boost::asio::buffer(&packet->header, sizeof(packet->header)),
                                             boost::asio::transfer_exactly(sizeof(packet->header)),
                                             ec);
    if (ec) {
        spdlog::error("Send packet header error: {}", ec.message());
    }

    //send payload
    if (packet->header.length > 0) {
        auto sentPayloadSize = boost::asio::write(socket,
                                                  boost::asio::buffer(packet->data, packet->header.length),
                                                  boost::asio::transfer_exactly(packet->header.length),
                                                  ec);
        if (ec) {
            spdlog::error("Send packet payload error: {}", ec.message());
            return false;
        }
    }

    return true;
}

// TODO set timeout
template <typename DataType>
bool Connection<DataType>::read(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& packet, boost::system::error_code& ec) {
    auto headerSize = boost::asio::read(socket,
                                        boost::asio::buffer(&packet->header, sizeof(packet->header)),
                                        boost::asio::transfer_exactly(sizeof(packet->header)),
                                        ec);

    if (ec) {
        spdlog::error("Read header error: {}", ec.message());
    }

    //read payload
    if (packet->header.length > 0) {
        auto DataSize =
            boost::asio::read(socket, boost::asio::buffer(packet->data, packet->header.length),
                              boost::asio::transfer_exactly(packet->header.length),
                              ec);

        if (ec) {
            spdlog::error("Read packet payload error: {}", ec.message());
            return false;
        }
    }
    return true;
}

} // namespace network

#endif  // NETWORKING_CONNECTION_H

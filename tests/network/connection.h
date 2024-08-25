//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_CONNECTION_H
#define NETWORKING_CONNECTION_H

#include "pch.h"

#include "defs.h"
#include "data_processor_interface.h"

namespace network {
// main goal for this class is to get/send packet from/to client
// DataProcessor is base class with implemented 2 functions:
    // waitNextData - wait available data for current queue (some other component should give it to us)
    // notifyComplete - give data that we got in processDataImpl to the next component
template <typename DataType>
class Connection: public DataProcessor<Connection<DataType>, DataType> {
    Connection(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator = (const Connection&) = delete;
    Connection& operator = (Connection&&) = delete;
public:
    // to start communicate we need socket
    explicit Connection(const Mode mode,
                        boost::asio::ip::tcp::socket socket,
                        std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                        std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue);

    // get data from socket (client)
    [[maybe_unused]] void processDataImpl();

    // read from socket to
    [[maybe_unused]] static bool read(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& data, boost::system::error_code& ec);
    [[maybe_unused]] static bool write(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& data, boost::system::error_code& ec);

    std::function<void(boost::asio::ip::tcp::socket& socket,
                       std::unique_ptr<DataType>& data,
                       boost::system::error_code& ec)> handler;

    static constexpr DataType ackPacket{{Header::Type::Ack}};
    static constexpr DataType nackPacket{{Header::Type::Nack}};
    std::unique_ptr<DataType> ackPackPtr;
    std::unique_ptr<DataType> nackPackPtr;

protected:
    boost::asio::ip::tcp::socket socket_;
    std::unique_ptr<DataType> data_;
};


template <typename DataType>
Connection<DataType>::Connection(const Mode mode,
                                 boost::asio::ip::tcp::socket socket,
                                 std::shared_ptr<ThreadSafeQueue<DataType>> currentQueue,
                                 std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
    :DataProcessor<Connection<DataType>, DataType>(currentQueue,nextQueue)
    ,socket_(std::move(socket))
    ,ackPackPtr(std::make_unique<DataType>(ackPacket))
    ,nackPackPtr(std::make_unique<DataType>(nackPacket)){

    if (mode == Mode::Server) {
        handler = [&](boost::asio::ip::tcp::socket& socket,
                      std::unique_ptr<DataType>& data,
                      boost::system::error_code& errorCode) {

            if (Connection<DataType>::read(socket, data, errorCode)) {
                // we got the package - send Acknowledgment Packet
                Connection<DataType>::write(socket, ackPackPtr, errorCode);
            } else {
                Connection<DataType>::write(socket, nackPackPtr, errorCode);
            }

            if (errorCode) {
                throw boost::system::system_error(errorCode);
            }
        };
    } else {
        handler = [&](boost::asio::ip::tcp::socket& socket,
                      std::unique_ptr<DataType>& data,
                      boost::system::error_code& errorCode) {

            if (Connection<DataType>::write(socket, data, errorCode)) {
                spdlog::error("Client can't send data");
            }
            // TODO handle nack packet
            if (Connection<DataType>::read(socket, data, errorCode)) {
                spdlog::error("Client didn't get Acknowledgment Packet");
            }

            if (errorCode) {
                throw boost::system::system_error(errorCode);
            }
        };
    }
}

template <typename DataType>
void Connection<DataType>::processDataImpl() {
    try {
        // TODO: think about place for errorCode
        boost::system::error_code errorCode;
        handler(socket_, data_, errorCode);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    // try catch
    // connectionHandler()
    // clientHandler [socket, packet] (){
    // create err
    // write/read(socket, packet, error)};
}

// TODO read write should work with any type: complex structures, stl containers or boost buffer
template <typename DataType>
bool Connection<DataType>::write(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& packet, boost::system::error_code& ec) {

    return false;
}

// TODO set timeout
template <typename DataType>
bool Connection<DataType>::read(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& packet, boost::system::error_code& ec) {
    // TODO: fix
    auto* raw_header_ptr = reinterpret_cast<char*>(&packet->header);

    auto headerSize = boost::asio::read(socket,
                                        boost::asio::buffer(raw_header_ptr, sizeof(packet->header)),
                                        boost::asio::transfer_exactly(sizeof(packet->header)),
                                        ec);

//    packet->header.length = raw_header_ptr->header.length;
    if (ec) {
        spdlog::error("Read header error: {}", ec.message());
        return false;
    }

    //read payload
    auto DataSize = boost::asio::read(socket,
                                      boost::asio::buffer(packet->data, packet->header.length),
                                      boost::asio::transfer_exactly(packet->header.length),
                                      ec);
    if (ec)
    {
        spdlog::error("Read packet payload error: {}", ec.message());
        return false;
    }

    return true;
}

} // namespace network

#endif  // NETWORKING_CONNECTION_H

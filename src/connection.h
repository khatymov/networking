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

const std::string ConnectionKey("Connection");

template <typename DataType>
class Connection: public std::enable_shared_from_this<Connection<DataType>>, public DataProcessor<Connection<DataType>, DataType> {
    Connection(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator = (const Connection&) = delete;
    Connection& operator = (Connection&&) = delete;
public:
    // for server
    explicit Connection(boost::asio::ip::tcp::socket socket,
                        NamedQueue<DataType> namedQueues);

    // for client
    explicit Connection(boost::asio::ip::tcp::socket socket,
                        QueuePtr<DataType> currentQueue,
                        QueuePtr<DataType> nextQueue);

    ~Connection() = default;

    // Client functions
    void processDataImpl();
    // read from/to socket
    // TODO: make them async
    [[maybe_unused]] static bool read(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& data, boost::system::error_code& ec);
    [[maybe_unused]] static bool write(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& data, boost::system::error_code& ec);

    // Server functions
    // run async read/write via socket (recursive call of functions below: [readHead-> readPayload-> writeHead(ack)-> readHead->..]
    void run();
    void readHead();
    void readPayload();
    void writeHead();

    // to be created once and reuse it
    std::unique_ptr<DataType> ackPackPtr = std::make_unique<DataType>(DataType({Header::Type::Ack}));
    std::unique_ptr<DataType> nackPackPtr = std::make_unique<DataType>(DataType({Header::Type::Nack}));

protected:
    // socket for reading/writing
    boost::asio::ip::tcp::socket socket_;
    // Session uses for server and includes other components, like decryptor and file writer
    std::shared_ptr<Session<DataType>> session;
    // define current mode
    const Mode mode_;
};

///////////////////////////////////////////////////////////////////////////////
////////////// SERVER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

template <typename DataType>
Connection<DataType>::Connection(boost::asio::ip::tcp::socket socket,
                                 NamedQueue<DataType> namedQueues)
    :DataProcessor<Connection<DataType>, DataType>(namedQueues[ConnectionKey].first, namedQueues[ConnectionKey].second)
    ,socket_(std::move(socket))
    ,session(std::make_shared<Session<DataType>>(namedQueues))
    ,mode_(Mode::Server) {
    session->handle();
}

template <typename DataType>
void Connection<DataType>::run() {
    assert(mode_== Mode::Server);
    readHead();
}

template <typename DataType>
void Connection<DataType>::readHead() {
    this->waitNextData();
    auto self(this->shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(&this->data_->header, sizeof(this->data_->header)),
    [this, self] (boost::system::error_code errorCode, std::size_t length) {
        if (!errorCode) {
            if (this->data_->header.type == Header::Type::Exit) {
                this->isProcessDone_ = true;
            }

            if (this->data_->header.length > 0) {
                readPayload();
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
            }
        } else {
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
             if (not this->isDone()) {
                 readHead();
             }
         } else {
             spdlog::error("error send header: {}", errorCode.message());
         }
     });
}

///////////////////////////////////////////////////////////////////////////////
////////////// CLIENT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

template <typename DataType>
Connection<DataType>::Connection(boost::asio::ip::tcp::socket socket,
                                 QueuePtr<DataType> currentQueue,
                                 QueuePtr<DataType> nextQueue)
    :DataProcessor<Connection<DataType>, DataType>(currentQueue, nextQueue)
      ,socket_(std::move(socket))
      ,mode_(Mode::Client) {

}

template <typename DataType>
void Connection<DataType>::processDataImpl() {
    assert(mode_== Mode::Client);
    try {
        boost::system::error_code errorCode;
        if (this->data_->header.type == Header::Type::Exit) {
            this->isProcessDone_ = true;
            spdlog::info("Client sends exit packet.");
        }

        if (not Connection<DataType>::write(socket_, this->data_, errorCode)) {
            spdlog::error("Client can't send data");
        }

        if (not Connection<DataType>::read(socket_, this->data_, errorCode)) {
            spdlog::error("Client didn't get Acknowledgment Packet");
        }

        if (this->data_->header.type == Header::Type::Nack) {
            spdlog::error("Client got Nack Packet");
        }

        if (errorCode) {
            throw boost::system::system_error(errorCode);
        }
    } catch (const boost::system::system_error& e) {
        std::cerr << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    } // add some others
}

// TODO read write should work with any type: complex structures, stl containers or boost buffer
template <typename DataType>
bool Connection<DataType>::write(boost::asio::ip::tcp::socket& socket, std::unique_ptr<DataType>& packet, boost::system::error_code& ec) {
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
            boost::asio::read(socket,
                              boost::asio::buffer(packet->data, packet->header.length),
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

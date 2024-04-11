/*! \file server.cpp
 * \brief Server class implementation.
 */


#include "server.h"
#include "socket_file_connection.h"

using namespace boost::asio;
using namespace boost::asio::ip;


Server::Server(boost::asio::io_context& io_context, const std::string& ip, const uint port)
    : m_acceptor(io_context, ip::tcp::endpoint(boost::asio::ip::make_address(ip), port)) {
    spdlog::info("starting [{}:{}]", ip, port);
    _waitForClientConnection();
}


void Server::_waitForClientConnection() {
    m_acceptor.async_accept([this] (boost::system::error_code errorCode, tcp::socket socket) {
        if (!errorCode) {
            spdlog::info("New connection");
            std::make_shared<SocketFileConnection>(std::move(socket))->run();
        } else {
            spdlog::error("Error has occurred during acceptance: {}", errorCode.message());
        }
        // Prime the asio context with more work - again simply wait for
        // another connection...
        _waitForClientConnection();
    });
}

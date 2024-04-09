/*! \file server.cpp
 * \brief Server class implementation.
 */


#include "server.h"
//#include "file_writer_connection.h"
#include "tmp_file_writer_connection.h"

using namespace boost::asio;
using namespace boost::asio::ip;

const uint threadNum = 1;

Server::Server(boost::asio::io_context& io_context, const std::string& ip, const uint port)
    : m_acceptor(io_context, ip::tcp::endpoint(boost::asio::ip::make_address(ip), port)) {
    spdlog::info("starting [{}:{}]", ip, port);
    waitForClientConnection();
}

//void Server::threadCallback(std::shared_ptr<io_context> ioContext) {
//    boost::system::error_code ec;
//    ioContext->run(ec);
//    if (ec)
//    {
//        spdlog::error("callback error: {}", ec.message());
//    }
//}
//void Server::start() {
//    // Launch the asio context in its own thread
//    waitForClientConnection();
////    m_ioContext.run();
////    m_threadContext = std::thread([this]() { m_ioContext->run(); });
//
//    while (true) {
//
//    }
////    for (uint i = 0; i < threadNum; ++i) {
////        //TODO do via lamda
////        m_executorsThreadGroup.create_thread(boost::bind(&Server::threadCallback, this, m_ioContext));
////    }
//}
//Server::~Server() {
//    stop();
//    spdlog::info("server stopped successfully");
//}

//void Server::stop() {
////    m_ioContext.stop();
//    // Tidy up the context thread
//    if (m_threadContext.joinable()) {
//        m_threadContext.join();
//    }
////    m_executorsThreadGroup.interrupt_all();
////    m_executorsThreadGroup.join_all();
//}
void Server::waitForClientConnection() {
    m_acceptor.async_accept([this] (boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            spdlog::info("New connection");
            std::make_shared<TmpFileWriterConnection>(std::move(socket))->run();
//            if (newConnection->ConnectToClient()) {
////                newConnection->FileProcess();
//                newConnection->run();
////                newConnection->isPingable();
//            }
        } else {
            spdlog::error("Error has occurred during acceptance: {}", ec.message());
        }

        // Prime the asio context with more work - again simply wait for
        // another connection...
        waitForClientConnection();
    });
}

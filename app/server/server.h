/*! \file server.h
 * \brief Server class interface.
 *
 * Class description.
 *
 */


#pragma once

#include "common.h"

using namespace boost::asio;
using namespace boost::asio::ip;
/*! \class Server
 * \brief Some briefing
 */
class Server
{
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server operator=(const Server&) = delete;
    Server operator=(Server&&) = delete;
public:

    //! \brief default constructor.
    Server(boost::asio::io_context& io_context, const std::string& ip, const uint port);

    //! \brief default destructor.
    ~Server() = default;

//    void start();
//    void stop();
private:
    void waitForClientConnection();
//    void threadCallback(std::shared_ptr<io_context> ioContext);
    //! List of private variables.
//    io_context m_ioContext;
//    tcp::endpoint m_endpoint;
    tcp::acceptor m_acceptor;
//    boost::thread_group m_executorsThreadGroup;
//    std::thread m_threadContext;
};

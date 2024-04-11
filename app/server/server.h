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
 * \brief Server accept client connection and starts communicate with a client
 */
class Server
{
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server operator=(const Server&) = delete;
    Server operator=(Server&&) = delete;

public:
    //! \brief Constructor accepts io_context - core of asio, ip and port to listen incoming connections
    Server(boost::asio::io_context& io_context, const std::string& ip, const uint port);
    //! \brief default destructor.
    ~Server() = default;
private:
    //private methods
    //! \brief Uses to accept new clients in async manner
    void _waitForClientConnection();

    //private variables
    //! \brief Handles new incoming connection attempts
    tcp::acceptor m_acceptor;
};

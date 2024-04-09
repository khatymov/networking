/*! \file connection_interface.h
 * \brief ConnectionInterface class interface.
 *
 * ConnectionInterface is responsible for reading(sending) messages from a socket.
 *
 */


#pragma once

#include "common.h"

/*! \class ConnectionInterface
 * \brief Interface with minimum functionality to accept messages and print them
 */
 //TODO: do not forget to add std::enable_shared_from_this<ConnectionInterface> for child classes
class ConnectionInterface  {
//    ConnectionInterface(const ConnectionInterface&) = delete;
//    ConnectionInterface(ConnectionInterface&&) = delete;
//    ConnectionInterface operator=(const ConnectionInterface&) = delete;
//    ConnectionInterface operator=(ConnectionInterface&&) = delete;
public:
    //! \brief accept io_context and socket.
    ConnectionInterface(boost::asio::io_context& asioContext, boost::asio::ip::tcp::socket socket);

    //! \brief Close socket if it is still opened
    virtual ~ConnectionInterface();
public:
    //! \brief Verify that socket is opened
    virtual bool ConnectToClient();
    //! \brief Receive and send packets until socket is opened
    virtual void Process();

    boost::asio::ip::tcp::socket &getSocket() noexcept;
    //! \brief This context is shared with the whole asio instance
    boost::asio::io_context& m_ioContext;

    boost::asio::ip::tcp::socket m_socket;

    //! \brief Each connection has a unique socket to a remote

};

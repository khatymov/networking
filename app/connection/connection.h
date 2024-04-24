/*! \file connection.h
 * \brief IConnection class is an interface.
 *
 * IConnection is responsible for reading(sending) messages from a socket.
 *
 */


#pragma once

#include "common.h"

/*! \class IConnection
 * \brief Just interface of a connection class
 */
class IConnection  {
    IConnection(const IConnection&) = delete;
    IConnection(IConnection&&) = delete;
    IConnection operator=(const IConnection&) = delete;
    IConnection operator=(IConnection&&) = delete;
public:
    //! \brief Default constructor for interface
    IConnection() = default;
    //! \brief Destructor for interface.
    virtual ~IConnection() = default;
    //! \brief Run connection and handle messages from i.e. socket
    virtual void run() = 0;
};

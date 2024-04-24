/*! \file client.h
 * \brief Client class interface.
 *
 * Class description.
 *
 */


#pragma once

#include <string>

#include "common.h"

/*! \class Client
 * \brief The purpose of this Client class is reading data
 * from a file and sending them via socket to server
 */
class Client {
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    Client operator=(const Client&) = delete;
    Client operator=(Client&&) = delete;
public:

    /**
     * \brief Constructor
     *
     * \param ip - the host that client tries to connect
     * \param port - port number
     */
    Client(const std::string& ip, const uint port);

    //! \brief default destructor.
    ~Client() = default;
    //! \brief connect via socket to a server
    [[nodiscard]] bool connect();
    /**
     * \brief Send file to a server
     *
     * \param fileName - the name of a file that should be sent
     * \return true if file sent
     */
    [[nodiscard]] bool sendFile(const std::string& fileName);

private:

    //! \brief asio context handles the data transfer...
    boost::asio::io_context m_context;
    //! \brief using to resolve hostname/ip-address into tangiable physical address
    boost::asio::ip::tcp::endpoint m_endpoint;
    //! \brief socket allows us to connect to the server
    boost::asio::ip::tcp::socket m_socket;
};

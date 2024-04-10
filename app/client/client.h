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
 * \brief Some briefing
 */
class Client {
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    Client operator=(const Client&) = delete;
    Client operator=(Client&&) = delete;
public:

    //! \brief default constructor.
    Client(const std::string& ip, const uint port);

    //! \brief default destructor.
    ~Client() = default;

    [[nodiscard]] bool connect();
    [[maybe_unused]] bool doPingPing();
    [[nodiscard]] bool sendFile(std::string& fileName);
private:
    void send(const Packet& packet);
    void receive(Packet& packet);

    void writeHeader(const Packet& packet);
    void writePayload(const Packet& packet);
    // asio context handles the data transfer...
    boost::asio::io_context m_context;
    boost::asio::ip::tcp::endpoint m_endpoint;
    boost::asio::ip::tcp::socket m_socket;
    //! List of private variables.
};

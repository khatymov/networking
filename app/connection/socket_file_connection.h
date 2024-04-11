/*! \file socket_file_connection.h
 * \brief SocketFileConnection class interface.
 *
 */


#pragma once

#include "common.h"
#include "connection.h"
#include "file_handler.h"


/*! \class SocketFileConnection
 * \brief The purpose of this class is to read a packet from a socket and write this data to a file
 */
class SocketFileConnection: public IConnection, public std::enable_shared_from_this<SocketFileConnection> {
    SocketFileConnection(const SocketFileConnection&) = delete;
    SocketFileConnection(SocketFileConnection&&) = delete;
    SocketFileConnection operator=(const SocketFileConnection&) = delete;
    SocketFileConnection operator=(SocketFileConnection&&) = delete;
public:

    //! \brief Constructor accepts a socket for communication with a client.
    SocketFileConnection(boost::asio::ip::tcp::socket socket);

    //! \brief default destructor.
    ~SocketFileConnection() = default;

    virtual void run() override;


private:
    //private methods

    //! \brief Read packet headers using async reading from a client via socket
    void _readHeader();
    //! \brief Read packet payloads using async reading from a client via socket
    void _readPayload();
    //! \brief Write packet headers using async writing to a client via socket
    void _writeHeader(const Packet& errorCode);
    //! \brief Write packet payloads using async writing to a client via socket
    void _writePayload(const Packet& errorCode);

    //! \brief According to type of a packet - use appropriate action
    void _handlePacket();

    //private variables
    //! \brief Each connection has a unique socket to a remote
    boost::asio::ip::tcp::socket m_socket;
    //! \brief Create a file and write data from a packet to a file
    FileHandler fileHandler;
    //! \brief This packet is using through the whole cycle of reading a header and a payload to prevent copying
    Packet _packet;

    Packet _ack_packet;
};

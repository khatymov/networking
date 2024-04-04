/*! \file connection.cpp
 * \brief Connection class implementation.
 */


#include "connection_interface.h"
#include "packet.h"

using namespace std;
using namespace boost::asio;

ConnectionInterface::ConnectionInterface(boost::asio::io_context& asioContext, boost::asio::ip::tcp::socket socket)
    : m_ioContext(asioContext),
      m_socket(std::move(socket)) {
}

ConnectionInterface::~ConnectionInterface()
{
    spdlog::info("~ConnectionInterface()");
    if (m_socket.is_open())
    {
        m_socket.shutdown(boost::asio::socket_base::shutdown_both);
        m_socket.close();
    }
}

bool ConnectionInterface::ConnectToClient()
{
    return m_socket.is_open();
}

void ConnectionInterface::Process()
{
    Packet packet;
    boost::system::error_code ec;
    while (m_socket.is_open()){
        size_t size = read(m_socket, boost::asio::buffer(&packet.payload, sizeof(packet.payload)), transfer_exactly(sizeof(packet.payload)), ec);

        if (!ec and size > 0) {
            spdlog::info("Client message: {}", packet.payload);
        } else {
            m_socket.close();
            if ((!ec and size == 0) or ec == boost::asio::error::eof) {
                spdlog::info("Finish reading, close socket");
            } else if (ec)
                spdlog::error("Error while reading client message: {}", ec.message());
        }
    }
}
ip::tcp::socket& ConnectionInterface::getSocket() noexcept
{
    return m_socket;
}

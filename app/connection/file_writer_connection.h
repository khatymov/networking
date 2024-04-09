/*! \file file_writer_connection.h
 * \brief FileWriterConnection class interface.
 *
 * The main purpose of this class is accept file data and write the data to a file
 *
 */


#pragma once

#include "connection_interface.h"
#include "file_handler.h"
/*! \class FileWriterConnection
 * \brief Some briefing
 */
class FileWriterConnection: public ConnectionInterface, std::enable_shared_from_this<FileWriterConnection> {
//    FileWriterConnection(const FileWriterConnection&) = delete;
//    FileWriterConnection(FileWriterConnection&&) = delete;
//    FileWriterConnection operator=(const FileWriterConnection&) = delete;
//    FileWriterConnection operator=(FileWriterConnection&&) = delete;
public:

    //! \brief default constructor.
    FileWriterConnection(boost::asio::io_context& ioContext, boost::asio::ip::tcp::socket socket);

    //! \brief default destructor.
    ~FileWriterConnection() override;

    //! \brief send/receive packets
    void Process() override;

    //! \brief send/receive packets
    void FileProcess();

    void run();

    bool isPingable();
private:

    void _ReadHeader();

    void _ReadPayload();

    void _ProcessPacket();


    FileHandler fileHandler;

    Packet _packet;
    //! List of private variables.
};

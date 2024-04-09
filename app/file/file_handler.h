/*! \file file_handler.h
 * \brief FileHandler class interface.
 *
 * Class description.
 *
 */


#pragma once

#include "file_interface.h"

#include "packet.h"

/*! \class FileHandler
 * \brief Some briefing
 */
class FileHandler: public FileInterface {
    FileHandler(const FileHandler&) = delete;
    FileHandler(FileHandler&&) = delete;
    FileHandler operator=(const FileHandler&) = delete;
    FileHandler operator=(FileHandler&&) = delete;
public:

    //! \brief default constructor.
    FileHandler();

    //! \brief default destructor.
    ~FileHandler();

    void read(Packet& packet);

    size_t write(Packet& packet);

    static bool isFileExist(const std::string& fileName);

    static std::string getUniqueName(const std::string& fileName);

    static std::string getFileHash(const std::string& fileName);
private:


    //! List of private variables.
};

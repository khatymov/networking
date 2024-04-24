/*! \file file_handler.h
 * \brief FileHandler class interface.
 *
 */


#pragma once

#include "file_interface.h"

#include "packet.h"

/*! \class FileHandler
 * \brief Read/write data to/from Packet and do some more
 */
class FileHandler: public FileInterface {
    FileHandler(const FileHandler&) = delete;
    FileHandler(FileHandler&&) = delete;
    FileHandler operator=(const FileHandler&) = delete;
    FileHandler operator=(FileHandler&&) = delete;
public:

    //! \brief default constructor.
    FileHandler() = default;
    //! \brief default destructor.
    ~FileHandler() = default;

    /**
     * \brief Read data to Packet
     *
     * \param packet - array that's used to fill the data from file.
     */
    void read(Packet& packet);
    /**
     * \brief Write data to Packet
     *
     * \param packet - array that's used to write the data to file.
     */
    size_t write(Packet& packet);
    /**
     * \brief Check that file that we are going to use is exist
     *
     * \param fileName - the name of a checking file
     * \return true - exist/ false - doesn't
     */
    static bool isFileExist(const std::string& fileName);
    /**
     * \brief Create unique name by adding timestamp
     *
     * \param fileName - the name of a file - the base for creating the unique name
     * \return unique name with added ts in ms
     */
    static std::string getUniqueName(const std::string& fileName);
    /**
     * \brief Generate a hash(sha256) based on file data
     *
     * \param fileName - the name of  afile
     * \return 64 byte hash
     */
    static std::string getFileHash(const std::string& fileName);
};

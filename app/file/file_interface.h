/*! \file file_interface.h
 * \brief FileInterface class interface.
 *
 * Class description.
 *
 */


#pragma once

#include <cstdio>
#include <string>
/*! \class FileInterface
 * \brief Some briefing
 */
class FileInterface {
    FileInterface(const FileInterface&) = delete;
    FileInterface(FileInterface&&) = delete;
    FileInterface operator=(const FileInterface&) = delete;
    FileInterface operator=(FileInterface&&) = delete;
public:

    //! \brief default constructor.
    FileInterface() = default;

    void open(const std::string& name, const std::string& mode);
    std::string getFilename() const;
    std::FILE* get();
    //! \brief virtual destructor - close opened file.
    virtual ~FileInterface();

private:

    std::FILE* m_file;
    std::string m_fileName;
    //! List of private variables.
};
